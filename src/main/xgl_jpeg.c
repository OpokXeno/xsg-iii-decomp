#include "common.h"
#include "shared.h"
/* The EE scratchpad (16 KB), where the GS queue and talk packets are built before they are sent. */
#define SCRATCHPAD_BASE 0x70000000
/* Pixel storage modes. */
#define GS_PSMCT32 0x00

typedef struct JpegImageDimensions {
    short width;
    short height;
} JpegImageDimensions;

/* Chroma subsampling of the first component (JPEG SOF sampling factors, H << 4 | V). */
typedef enum JpegSamplingMode {
    JPEG_SAMPLING_444,
    JPEG_SAMPLING_422,
    JPEG_SAMPLING_420
} JpegSamplingMode;

/* The 32-byte decode request. With destination 0 the image is uploaded to
 * the GS frame buffer at frameBufferBase / frameBufferWidth, origin
 * originX / originY; otherwise it is written to destination. */
typedef struct JpegDecodeControl {
    u8 *source;
    u8 *destination;
    int consumedSize; /* +0x08: bytes of source parsed up to the RST0/EOI marker */
    short frameBufferBase;
    short frameBufferWidth;
    u16 originX;
    u16 originY;
    JpegImageDimensions imageLimits; /* +0x14 */
    u8 luminancePercent;
    u8 chrominancePercent;
    u8 scaleMode; /* +0x1a */
    u8 pixelFormat;
    u8 redPercent;
    u8 greenPercent;
    u8 bluePercent;
    u8 unmodeled_1f;
} JpegDecodeControl;

int xglJpegDecode(JpegDecodeControl *request);
int F2I(float value);
float I2F(int value);
extern u8 *buffer;
extern void *memset(void *destination, int value, unsigned int count);

/* JPEG block geometry: 8x8 coefficient blocks, six per MCU (up to four Y, then Cb, Cr). */
#define JPEG_BLOCK_DIM 8
#define JPEG_BLOCK_COEFFS (JPEG_BLOCK_DIM * JPEG_BLOCK_DIM)
#define JPEG_MCU_BLOCKS 6
#define JPEG_BLOCK_CB 4
#define JPEG_BLOCK_CR 5
#define JPEG_MCU_PIXELS 16
/* One decoded pixel is a float Y/Cb/Cr triple; a 16-pixel MCU row holds 48 floats. */
#define JPEG_YUV_ROW_FLOATS (JPEG_MCU_PIXELS * 3)
#define JPEG_LEVEL_SHIFT 128.0f
#define JPEG_SAMPLING_FACTORS_444 0x11
#define JPEG_SAMPLING_FACTORS_422 0x21
#define JPEG_SAMPLING_FACTORS_420 0x22
/* Marker bytes that follow the 0xFF prefix. */
#define JPEG_MARKER_PREFIX 0xFF
#define JPEG_MARKER_SOF0 0xC0
#define JPEG_MARKER_DHT 0xC4
#define JPEG_MARKER_RST0 0xD0
#define JPEG_MARKER_SOI 0xD8
#define JPEG_MARKER_EOI 0xD9
#define JPEG_MARKER_SOS 0xDA
#define JPEG_MARKER_DQT 0xDB
#define JPEG_MARKER_DRI 0xDD
/* Huffman tables: 16 code lengths; decoder tables use 0xFE/0xFF boundary entries. */
#define JPEG_HUFF_LENGTHS 16
#define JPEG_HUFF_LENGTH_END 0xFE
#define JPEG_HUFF_TABLE_END 0xFF
#define JPEG_DHT_DC_LUMA 0x00
#define JPEG_DHT_DC_CHROMA 0x01
#define JPEG_DHT_AC_LUMA 0x10
#define JPEG_AC_ZRL 0xF0 /* 16 zeros in a row */
#define JPEG_ZRL_RUN 16
/* The component an 8x8 block belongs to (selects the Huffman and quantization tables). */
#define JPEG_COMPONENT_Y 0
#define JPEG_COMPONENT_CB 1
#define JPEG_COMPONENT_CR 2
#define JPEG_STAGING_ADDRESS (SCRATCHPAD_BASE + 0x3000)
/* IJG-style quality to quantizer scaling. */
#define JPEG_QUALITY_MAX 100.0f
#define JPEG_QUALITY_KNEE 50.0f
#define JPEG_QUALITY_LOW_NUMERATOR 5000.0f
#define JPEG_QUALITY_HIGH_BASE 200.0f
#define JPEG_QUANT_ROUNDING 50.0f
#define JPEG_PERCENT_SCALE 0.01f
#define JPEG_QUANT_MAX 255

typedef struct JpegYuvPixel {
    float luminance;
    float firstChroma;
    float secondChroma;
    u8 unmodeled_0c[4];
} JpegYuvPixel;

typedef struct JpegHuffCode {
    u16 code;
    s16 length;
} JpegHuffCode;

/* A Huffman table buffer is written as code/length entries for the encoder and as
   code/symbol pairs with 0xFE/0xFF boundaries for the decoder. */
typedef union JpegHuffDcTable {
    JpegHuffCode encode[32];
    s16 decode[64];
} JpegHuffDcTable;

typedef union JpegHuffAcTable {
    JpegHuffCode encode[512];
    s16 decode[1024];
} JpegHuffAcTable;

/* Six 8x8 float blocks of MCU coefficients, addressed flat or block by block. */
typedef union JpegMcuPlanes {
    float planes[JPEG_MCU_BLOCKS * JPEG_BLOCK_COEFFS];
    float blocks[JPEG_MCU_BLOCKS][JPEG_BLOCK_DIM][JPEG_BLOCK_DIM];
} JpegMcuPlanes;

/* One 4 KiB buffer read as YUV pixels, as float planes, and as quantized int blocks. */
typedef union JpegDecodedYuv {
    JpegYuvPixel pixels[JPEG_MCU_PIXELS * JPEG_MCU_PIXELS];
    float floats[JPEG_MCU_PIXELS * JPEG_MCU_PIXELS * 3];
    int blocks[16][JPEG_BLOCK_COEFFS];
} JpegDecodedYuv;

/* The codec work area is quadword aligned for FFIDCT's VU0 transfers.
 * Quantization tables occupy +0xD0 and +0x1D0; bit input, IDCT storage and
 * restart counters are modeled below. Other regions remain explicit partial
 * spans. */
typedef struct JpegWork {
    u8 unmodeled_0000[0xD0];
    float luminanceQuantization[JPEG_BLOCK_COEFFS];    /* +0xD0 */
    float chrominanceQuantization[JPEG_BLOCK_COEFFS];  /* +0x1D0 */
    /* +0x2D0: the encoder's 8-bit quantizer values (luminance, chrominance),
       rebuilt from the quality and written to the DQT segments. */
    u8 quantizerValues[2][JPEG_BLOCK_COEFFS];
    u8 *buffer;
    int outputPosition;
    signed char bitsRemaining;
    u8 bufferedByte;
    u8 unmodeled_035a[0x360 - 0x35a];
    int samplingMode;
    short imageWidth;
    short imageHeight;
    short maximumX; /* +0x368 */
    short maximumY; /* +0x36a */
    int rowBytes;
    JpegHuffDcTable luminanceDcCodes; /* +0x370 */
    JpegHuffDcTable chrominanceDcCodes; /* +0x3f0 */
    JpegHuffAcTable luminanceAcCodes; /* +0x470 */
    JpegHuffAcTable chrominanceAcCodes; /* +0xc70 */
    float idctBlock[JPEG_BLOCK_DIM][JPEG_BLOCK_DIM];
    float idctStage[JPEG_BLOCK_DIM];
    int dcPredictors[3]; /* +0x1590 */
    unsigned char unmodeled_159c[0x15a0 - 0x159c];
    JpegDecodedYuv decodedYuvPixels; /* +0x15a0 */
    JpegMcuPlanes decodedMcuPlanes; /* +0x25a0 */
    u32 restartInterval;
    u32 restartIntervalPlusOne;
    int colorScaleMode; /* +0x2ba8 */
    float luminanceScale; /* +0x2bac */
    float firstChromaScale; /* +0x2bb0 */
    float secondChromaScale; /* +0x2bb4 */
    float redScale; /* +0x2bb8 */
    float greenScale; /* +0x2bbc */
    float blueScale; /* +0x2bc0 */
} JpegWork;

static void PutBits(u16 code, int length);
int GetBit(void);
extern u8 LQTableOrg_1[JPEG_BLOCK_COEFFS];
extern u8 CQTableOrg_2[JPEG_BLOCK_COEFFS];
JpegWork *sw = (JpegWork *) SCRATCHPAD_BASE;
extern u8 ZigZag[JPEG_BLOCK_COEFFS];
/* DCT basis: row k holds the eight cosine weights of frequency k. */
extern float dctCosineTable[JPEG_BLOCK_DIM][JPEG_BLOCK_DIM];
u8 *DefineQuantizeTable(u8 *marker);

/* JpegDecodeControl.pixelFormat bit 0: the destination takes RGB555 instead of RGBA32 texels. */
#define JPEG_PIXEL_FORMAT_RGB555 1
#define JPEG_RGBA_BYTES 4
#define JPEG_RGB555_BYTES 2

void sceGsSetDefLoadImage(void *image, short fbp, short fbw, short psm, short x, short y, short w, short h);
void sceGsExecLoadImage(void *image, void *data);
void *memcpy(void *destination, const void *source, unsigned int size);
/* VU0 constants of the IDCT: imm0_3 holds the eight basis rows (vf1-vf8) and the two butterfly
 * vectors (vf27/vf28), imm1_4 the six colour-conversion rows (vf21-vf26). */
extern float imm0_3[];
extern u32 imm1_4[];
u8 *StartOfScan(u8 *segment, JpegDecodeControl *requestAddress);

#define JPEG_SCALE_YUV 1
#define JPEG_SCALE_RGB 2
/* Maximum coordinate stored for an image without size limits. */
#define JPEG_NO_LIMIT 0x7FFF

extern u8 ZigZag[64];
extern float dctCosineTable[8][8];
extern u8 LQTableOrg_1[64];
extern u8 CQTableOrg_2[64];
extern float imm0_3[40];
extern u32 imm1_4[24];

/* Input rows contain sixteen consecutive Y/U/V float triples. */
void ConvertYUV2MCU(float *planes, const float *pixels, int samplingMode)
{
    int row;
    int column;

    switch (samplingMode) {
    case JPEG_SAMPLING_444:
        for (row = 0; row < JPEG_BLOCK_DIM; row++) {
            for (column = 0; column < JPEG_BLOCK_DIM; column++) {
                planes[row * JPEG_BLOCK_DIM + column] = pixels[row * JPEG_YUV_ROW_FLOATS+ column * 3] - JPEG_LEVEL_SHIFT;
                planes[JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column] = pixels[row * JPEG_YUV_ROW_FLOATS+ column * 3 + 1];
                planes[JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column] = pixels[row * JPEG_YUV_ROW_FLOATS+ column * 3 + 2];
            }
        }
        return;
    case JPEG_SAMPLING_422:
        for (row = 0; row < JPEG_BLOCK_DIM; row++) {
            for (column = 0; column < JPEG_BLOCK_DIM; column++) {
                planes[row * JPEG_BLOCK_DIM + column] = pixels[row * JPEG_YUV_ROW_FLOATS+ column * 3] - JPEG_LEVEL_SHIFT;
                planes[JPEG_BLOCK_COEFFS + row * JPEG_BLOCK_DIM + column] = pixels[row * JPEG_YUV_ROW_FLOATS+ column * 3 + 24] - JPEG_LEVEL_SHIFT;
                planes[JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column] =
                    (pixels[row * JPEG_YUV_ROW_FLOATS+ column * 6 + 1] +
                     pixels[row * JPEG_YUV_ROW_FLOATS+ column * 6 + 4]) * 0.5f;
                planes[JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column] =
                    (pixels[row * JPEG_YUV_ROW_FLOATS+ column * 6 + 2] +
                     pixels[row * JPEG_YUV_ROW_FLOATS+ column * 6 + 5]) * 0.5f;
            }
        }
        return;
    case JPEG_SAMPLING_420:
    default:
        for (row = 0; row < JPEG_BLOCK_DIM; row++) {
            for (column = 0; column < JPEG_BLOCK_DIM; column++) {
                planes[row * JPEG_BLOCK_DIM + column] = pixels[row * JPEG_YUV_ROW_FLOATS+ column * 3] - JPEG_LEVEL_SHIFT;
                planes[JPEG_BLOCK_COEFFS + row * JPEG_BLOCK_DIM + column] = pixels[row * JPEG_YUV_ROW_FLOATS+ column * 3 + 24] - JPEG_LEVEL_SHIFT;
                planes[2 * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column] = pixels[row * JPEG_YUV_ROW_FLOATS+ column * 3 + 384] - JPEG_LEVEL_SHIFT;
                planes[3 * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column] = pixels[row * JPEG_YUV_ROW_FLOATS+ column * 3 + 408] - JPEG_LEVEL_SHIFT;
                planes[JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column] =
                    (pixels[row * 96 + column * 6 + 1] +
                     pixels[row * 96 + column * 6 + 4] +
                     pixels[row * 96 + column * 6 + 49] +
                     pixels[row * 96 + column * 6 + 52]) * 0.25f;
                planes[JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column] =
                    (pixels[row * 96 + column * 6 + 2] +
                     pixels[row * 96 + column * 6 + 5] +
                     pixels[row * 96 + column * 6 + 50] +
                     pixels[row * 96 + column * 6 + 53]) * 0.25f;
            }
        }
        break;
    }
}

/* Forward 8x8 DCT as two matrix passes over the cosine table: rows of the
 * block into idctBlock, then columns of idctBlock back into the block,
 * scaled by 1/8. XS2 replaces XS1's unrolled butterfly weights with this
 * table form. */
void FFDCT(float *block)
{
    JpegWork *work = sw;
    int row;
    int column;
    int frequency;

    for (row = 0; row < JPEG_BLOCK_DIM; row++) {
        for (frequency = 0; frequency < JPEG_BLOCK_DIM; frequency++) {
            sw->idctBlock[row][frequency] =
                block[row * JPEG_BLOCK_DIM + 0] * dctCosineTable[frequency][0] +
                block[row * JPEG_BLOCK_DIM + 1] * dctCosineTable[frequency][1] +
                block[row * JPEG_BLOCK_DIM + 2] * dctCosineTable[frequency][2] +
                block[row * JPEG_BLOCK_DIM + 3] * dctCosineTable[frequency][3] +
                block[row * JPEG_BLOCK_DIM + 4] * dctCosineTable[frequency][4] +
                block[row * JPEG_BLOCK_DIM + 5] * dctCosineTable[frequency][5] +
                block[row * JPEG_BLOCK_DIM + 6] * dctCosineTable[frequency][6] +
                block[row * JPEG_BLOCK_DIM + 7] * dctCosineTable[frequency][7];
        }
    }
    for (column = 0; column < JPEG_BLOCK_DIM; column++) {
        for (frequency = 0; frequency < JPEG_BLOCK_DIM; frequency++) {
            block[frequency * JPEG_BLOCK_DIM + column] =
                (work->idctBlock[0][column] * dctCosineTable[frequency][0] +
                work->idctBlock[1][column] * dctCosineTable[frequency][1] +
                work->idctBlock[2][column] * dctCosineTable[frequency][2] +
                work->idctBlock[3][column] * dctCosineTable[frequency][3] +
                work->idctBlock[4][column] * dctCosineTable[frequency][4] +
                work->idctBlock[5][column] * dctCosineTable[frequency][5] +
                work->idctBlock[6][column] * dctCosineTable[frequency][6] +
                work->idctBlock[7][column] * dctCosineTable[frequency][7]) * 0.125f;
        }
    }
}

void Quantize(int *quantized, const float *coefficients, const float *reciprocals)
{
    float coefficient;
    float reciprocal;
    float scaled;
    int index;

    for (index = JPEG_BLOCK_COEFFS - 1; index >= 0; index--) {
        coefficient = *coefficients;
        coefficients++;
        reciprocal = *reciprocals;
        reciprocals++;
        scaled = coefficient * reciprocal;
        if (coefficient < 0.0f) {
            scaled -= 0.5f;
        } else {
            scaled += 0.5f;
        }
        *quantized = F2I(scaled);
        quantized++;
    }
}

void RebuildQuantizeTable(u8 *quantizers, float *reciprocals,
                                 int chrominance, int quality)
{
    const u8 *original;
    float *reciprocalDest;
    float scale;
    int index;
    int value;

    if (chrominance == 0) {
        original = LQTableOrg_1;
    } else {
        original = CQTableOrg_2;
    }
    scale = (float) quality;
    if (scale <= 0.0f) {
        scale = 1.0f;
    }
    if (scale > JPEG_QUALITY_MAX) {
        scale = JPEG_QUALITY_MAX;
    }
    if (scale < JPEG_QUALITY_KNEE) {
        scale = JPEG_QUALITY_LOW_NUMERATOR / scale;
    } else {
        scale = JPEG_QUALITY_HIGH_BASE - 2.0f * scale;
    }
    reciprocalDest = reciprocals;
    for (index = 0; index < JPEG_BLOCK_COEFFS; index++) {
        value = F2I(((float) original[index] * scale + JPEG_QUANT_ROUNDING) * JPEG_PERCENT_SCALE);
        if (value <= 0) {
            value = 1;
        }
        if (value >= JPEG_QUANT_MAX + 1) {
            value = JPEG_QUANT_MAX;
        }
        quantizers[index] = value;
        *reciprocalDest++ = 1.0f / I2F(value);
    }
}

static void PutBits(u16 code, int length)
{
    u8 *output = sw->buffer;
    int bits = code;
    int bufferedBits;
    JpegWork *work;

    if (length != 0) {
        bits &= (1 << length) - 1;
        while (length + sw->bitsRemaining >= 8) {
            sw->bufferedByte |= bits >> (length + sw->bitsRemaining - 8);
            output[sw->outputPosition] = sw->bufferedByte;
            if (sw->bufferedByte == JPEG_MARKER_PREFIX) {
                sw->outputPosition++;
                output[sw->outputPosition] = 0;
            }
            work = sw;
            bufferedBits = work->bitsRemaining;
            work->bitsRemaining = 0;
            length = length + bufferedBits - 8;
            work->outputPosition++;
            sw->bufferedByte = 0;
        }
        if (length > 0) {
            sw->bufferedByte |= bits << (8 - sw->bitsRemaining - length);
            sw->bitsRemaining += length;
        }
    }
}

static void EncodeDc(int value, JpegHuffCode *table)
{
    int magnitude;
    int negative;
    int remaining;
    int length;
    u16 bits;

    magnitude = value;
    if (magnitude == 0) {
        PutBits(table[0].code, table[0].length);
        return;
    }
    negative = 0;
    if (magnitude < 0) {
        negative = 1;
        magnitude = -magnitude;
    }
    remaining = magnitude;
    length = 0;
    while (remaining != 0) {
        remaining >>= 1;
        length++;
    }
    PutBits(table[length].code, table[length].length);
    if (negative) {
        bits = ~magnitude & 0xFFFF;
    } else {
        bits = magnitude & 0xFFFF;
    }
    PutBits(bits, length);
}

static void EncodeAc(int value, int runIndex, JpegHuffCode *table)
{
    int magnitude;
    int negative;
    int remaining;
    int length;
    u16 bits;

    magnitude = value;
    if (magnitude == 0) {
        PutBits(table[runIndex].code, table[runIndex].length);
        return;
    }

    negative = 0;
    if (magnitude < 0) {
        negative = 1;
        magnitude = -magnitude;
    }

    remaining = magnitude;
    length = 0;
    while (remaining != 0) {
        remaining >>= 1;
        length++;
    }

    PutBits(table[length + runIndex].code, table[length + runIndex].length);

    if (negative) {
        bits = ~magnitude & 0xFFFF;
    } else {
        bits = magnitude & 0xFFFF;
    }
    PutBits(bits, length);
}

void HuffmanEncode(int *coefficients, JpegHuffCode *dcCodes,
                         JpegHuffCode *acCodes, int *previousDc)
{
    short difference;
    short value;
    short zeroRun;
    int index;

    zeroRun = 0;
    index = 1;
    difference = (short) (coefficients[0] - *previousDc);
    *previousDc += difference;
    EncodeDc(difference, dcCodes);

    for (;;) {
        value = (short) coefficients[ZigZag[index]];
        index++;
        if (value == 0) {
            if (index >= JPEG_BLOCK_COEFFS) {
                EncodeAc(0, 0, acCodes);
                break;
            }
            zeroRun++;
        } else {
            while (zeroRun >= JPEG_ZRL_RUN) {
                EncodeAc(0, JPEG_AC_ZRL, acCodes);
                zeroRun -= JPEG_ZRL_RUN;
            }
            EncodeAc(value, zeroRun * 16, acCodes);
            if (index >= JPEG_BLOCK_COEFFS) {
                break;
            }
            zeroRun = 0;
        }
    }
}

void ExtractHuffmanTableSub(JpegHuffCode *table, const u8 *symbols, int tableSize)
{
    const u8 *counts;
    int code;
    s16 length;
    int lengthsRemaining;
    int symbolsRemaining;
    JpegHuffCode *entry;

    memset(table, 0, tableSize * 4);
    counts = symbols;
    code = 0;
    length = 1;
    symbols += JPEG_HUFF_LENGTHS;
    for (lengthsRemaining = JPEG_HUFF_LENGTHS - 1; lengthsRemaining >= 0; lengthsRemaining--) {
        symbolsRemaining = *counts++;
        while (symbolsRemaining-- > 0) {
            entry = &table[*symbols++];
            entry->code = code;
            code = (code + 1) & 0xFFFF;
            entry->length = length;
        }
        code = (code * 2) & 0xFFFE;
        length++;
    }
}

#include "main/res_get_path.h"
#define GS_PSMCT24 0x01
/* Layout of headerALL, the canned JFIF/DQT/SOF0/DHT/SOS header written by the encoder. */
#define JPEG_HEADER_SIZE 0x26F
#define JPEG_HEADER_DENSITY_OFFSET 14
#define JPEG_HEADER_DQT_LUMA_OFFSET 25
#define JPEG_HEADER_DQT_CHROMA_OFFSET 94
#define JPEG_HEADER_SOF_SIZE_OFFSET 163 /* height, width, then component sampling at +6 */
#define JPEG_HEADER_DHT_DC_LUMA_OFFSET 0xB6
#define JPEG_HEADER_DHT_AC_LUMA_OFFSET 0xD7
#define JPEG_HEADER_DHT_DC_CHROMA_OFFSET 0x18E
#define JPEG_HEADER_DHT_AC_CHROMA_OFFSET 0x1AF
#define JPEG_HUFF_DC_SYMBOLS 16
#define JPEG_HUFF_AC_SYMBOLS 256
#define JPEG_DENSITY_HEIGHT_NUMERATOR 0xAF00
#define JPEG_DENSITY_WIDTH_NUMERATOR 0xFA00
/* sceGsSetDefStoreImage's packet sits at this offset of the work area. */
#define JPEG_STORE_IMAGE_OFFSET 0x60
/* RGB to YCbCr (JFIF). */
#define JPEG_Y_FROM_R 0.299f
#define JPEG_Y_FROM_G 0.587f
#define JPEG_Y_FROM_B 0.114f
#define JPEG_CB_FROM_R -0.16874f
#define JPEG_CB_FROM_G -0.33126f
#define JPEG_CB_FROM_B 0.5f
#define JPEG_CR_FROM_R 0.5f
#define JPEG_CR_FROM_G -0.41869f
#define JPEG_CR_FROM_B -0.08131f

extern u8 headerALL[];
int sceGsSetDefStoreImage(void *image, short fbp, short fbw, short psm, short x, short y,
                          unsigned int w, unsigned int h);
int sceGsExecStoreImage(void *image, void *buffer);

int xglJpegEncode(JpegEncodeRequest *request)
{
    u8 *header;
    u8 *outputBuffer;
    int y;
    int x;
    int row;
    int column;
    int i;
    int mcuWidth;
    int mcuHeight;
    int quotient;

    sw->rowBytes = request->width * 3;
    RebuildQuantizeTable(sw->quantizerValues[0], sw->luminanceQuantization, 0, request->luminanceQuality);
    RebuildQuantizeTable(sw->quantizerValues[1], sw->chrominanceQuantization, 1,
                         request->chrominanceQuality);
    ExtractHuffmanTableSub(sw->luminanceDcCodes.encode, headerALL + JPEG_HEADER_DHT_DC_LUMA_OFFSET,
                           JPEG_HUFF_DC_SYMBOLS);
    ExtractHuffmanTableSub(sw->chrominanceDcCodes.encode, headerALL + JPEG_HEADER_DHT_DC_CHROMA_OFFSET,
                           JPEG_HUFF_DC_SYMBOLS);
    ExtractHuffmanTableSub(sw->luminanceAcCodes.encode, headerALL + JPEG_HEADER_DHT_AC_LUMA_OFFSET,
                           JPEG_HUFF_AC_SYMBOLS);
    ExtractHuffmanTableSub(sw->chrominanceAcCodes.encode, headerALL + JPEG_HEADER_DHT_AC_CHROMA_OFFSET,
                           JPEG_HUFF_AC_SYMBOLS);

    sw->buffer = request->outputBuffer;
    sw->bitsRemaining = 0;
    sw->bufferedByte = 0;
    __builtin_memcpy(sw->buffer, headerALL, JPEG_HEADER_SIZE);

    sw->outputPosition = JPEG_HEADER_SIZE;
    header = sw->buffer + JPEG_HEADER_DENSITY_OFFSET;
    quotient = JPEG_DENSITY_HEIGHT_NUMERATOR / request->height;
    header[1] = quotient;
    header[0] = quotient >> 8;
    quotient = JPEG_DENSITY_WIDTH_NUMERATOR / request->width;
    header[3] = quotient;
    header[2] = quotient >> 8;

    header = sw->buffer + JPEG_HEADER_DQT_LUMA_OFFSET;
    for (i = 0; i < JPEG_BLOCK_COEFFS; i++) {
        *header = sw->quantizerValues[0][ZigZag[i]];
        header++;
    }
    header = sw->buffer + JPEG_HEADER_DQT_CHROMA_OFFSET;
    for (i = 0; i < JPEG_BLOCK_COEFFS; i++) {
        *header = sw->quantizerValues[1][ZigZag[i]];
        header++;
    }

    switch (request->samplingMode) {
    case JPEG_SAMPLING_444:
        i = JPEG_SAMPLING_FACTORS_444;
        break;
    case JPEG_SAMPLING_422:
        i = JPEG_SAMPLING_FACTORS_422;
        break;
    case JPEG_SAMPLING_420:
    default:
        i = JPEG_SAMPLING_FACTORS_420;
        break;
    }
    header = sw->buffer + JPEG_HEADER_SOF_SIZE_OFFSET;
    header[0] = request->height >> 8;
    header[1] = request->height;
    header[2] = request->width >> 8;
    header[3] = request->width;
    header[6] = i;

    sw->dcPredictors[JPEG_COMPONENT_CR] = 0;
    sw->dcPredictors[JPEG_COMPONENT_CB] = 0;
    sw->dcPredictors[JPEG_COMPONENT_Y] = 0;

    switch (request->samplingMode) {
    case JPEG_SAMPLING_444:
        mcuWidth = JPEG_BLOCK_DIM;
        mcuHeight = mcuWidth;
        break;
    case JPEG_SAMPLING_422:
        mcuWidth = JPEG_MCU_PIXELS;
        mcuHeight = JPEG_BLOCK_DIM;
        break;
    case JPEG_SAMPLING_420:
    default:
        mcuWidth = JPEG_MCU_PIXELS;
        mcuHeight = mcuWidth;
        break;
    }

    for (y = 0; y < request->height; y += mcuHeight) {
        for (x = 0; x < request->width; x += mcuWidth) {
            void *storeImage = &sw->unmodeled_0000[JPEG_STORE_IMAGE_OFFSET];

            sceGsSetDefStoreImage(storeImage, request->frameBufferBase, request->frameBufferWidth,
                                  GS_PSMCT24, request->originX + x, request->originY + y,
                                  /* Zero-extended with sll/srl as in the original; a (u16) cast
                                     compiles to andi and moves the argument loads. */
                                  ((unsigned) mcuWidth << 16) >> 16, ((unsigned) mcuHeight << 16) >> 16);
            sceGsExecStoreImage(&sw->unmodeled_0000[JPEG_STORE_IMAGE_OFFSET], (void *) JPEG_STAGING_ADDRESS);

            for (row = 0; row < mcuHeight; row++) {
                u8 *rgb = (u8 *) JPEG_STAGING_ADDRESS + row * mcuWidth * 3;
                float *pixel = sw->decodedYuvPixels.floats + row * JPEG_YUV_ROW_FLOATS;
                for (column = 0; column < mcuWidth; column++) {
                    float red = I2F(rgb[0]);
                    float green = I2F(rgb[1]);
                    float blue = I2F(rgb[2]);

                    pixel[0] = red * JPEG_Y_FROM_R + green * JPEG_Y_FROM_G + blue * JPEG_Y_FROM_B;
                    pixel[1] = red * JPEG_CB_FROM_R + green * JPEG_CB_FROM_G + blue * JPEG_CB_FROM_B;
                    pixel[2] = red * JPEG_CR_FROM_R + green * JPEG_CR_FROM_G + blue * JPEG_CR_FROM_B;
                    rgb += 3;
                    pixel += 3;
                }
            }

            ConvertYUV2MCU(sw->decodedMcuPlanes.planes, sw->decodedYuvPixels.floats, request->samplingMode);

            switch (request->samplingMode) {
        case JPEG_SAMPLING_444:
            FFDCT(sw->decodedMcuPlanes.planes + 0);
            FFDCT(sw->decodedMcuPlanes.planes + JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS);
            FFDCT(sw->decodedMcuPlanes.planes + JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS);
            Quantize(sw->decodedYuvPixels.blocks[0], sw->decodedMcuPlanes.planes + 0, sw->luminanceQuantization);
            Quantize(sw->decodedYuvPixels.blocks[JPEG_BLOCK_CB],
                     sw->decodedMcuPlanes.planes + JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS, sw->chrominanceQuantization);
            Quantize(sw->decodedYuvPixels.blocks[JPEG_BLOCK_CR],
                     sw->decodedMcuPlanes.planes + JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS, sw->chrominanceQuantization);
            HuffmanEncode(sw->decodedYuvPixels.blocks[0], sw->luminanceDcCodes.encode,
                          sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
            HuffmanEncode(sw->decodedYuvPixels.blocks[JPEG_BLOCK_CB], sw->chrominanceDcCodes.encode,
                          sw->chrominanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_CB]);
            HuffmanEncode(sw->decodedYuvPixels.blocks[JPEG_BLOCK_CR], sw->chrominanceDcCodes.encode,
                          sw->chrominanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_CR]);
            break;
        case JPEG_SAMPLING_422:
            FFDCT(sw->decodedMcuPlanes.planes + 0);
            FFDCT(sw->decodedMcuPlanes.planes + JPEG_BLOCK_COEFFS);
            FFDCT(sw->decodedMcuPlanes.planes + JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS);
            FFDCT(sw->decodedMcuPlanes.planes + JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS);
            Quantize(sw->decodedYuvPixels.blocks[0], sw->decodedMcuPlanes.planes + 0, sw->luminanceQuantization);
            Quantize(sw->decodedYuvPixels.blocks[1], sw->decodedMcuPlanes.planes + JPEG_BLOCK_COEFFS,
                     sw->luminanceQuantization);
            Quantize(sw->decodedYuvPixels.blocks[JPEG_BLOCK_CB],
                     sw->decodedMcuPlanes.planes + JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS, sw->chrominanceQuantization);
            Quantize(sw->decodedYuvPixels.blocks[JPEG_BLOCK_CR],
                     sw->decodedMcuPlanes.planes + JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS, sw->chrominanceQuantization);
            HuffmanEncode(sw->decodedYuvPixels.blocks[0], sw->luminanceDcCodes.encode,
                          sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
            HuffmanEncode(sw->decodedYuvPixels.blocks[1], sw->luminanceDcCodes.encode,
                          sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
            HuffmanEncode(sw->decodedYuvPixels.blocks[JPEG_BLOCK_CB], sw->chrominanceDcCodes.encode,
                          sw->chrominanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_CB]);
            HuffmanEncode(sw->decodedYuvPixels.blocks[JPEG_BLOCK_CR], sw->chrominanceDcCodes.encode,
                          sw->chrominanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_CR]);
            break;
        case JPEG_SAMPLING_420:
        default:
            FFDCT(sw->decodedMcuPlanes.planes + 0);
            FFDCT(sw->decodedMcuPlanes.planes + JPEG_BLOCK_COEFFS);
            FFDCT(sw->decodedMcuPlanes.planes + 2 * JPEG_BLOCK_COEFFS);
            FFDCT(sw->decodedMcuPlanes.planes + 3 * JPEG_BLOCK_COEFFS);
            FFDCT(sw->decodedMcuPlanes.planes + JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS);
            FFDCT(sw->decodedMcuPlanes.planes + JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS);
            Quantize(sw->decodedYuvPixels.blocks[0], sw->decodedMcuPlanes.planes + 0, sw->luminanceQuantization);
            Quantize(sw->decodedYuvPixels.blocks[1], sw->decodedMcuPlanes.planes + JPEG_BLOCK_COEFFS,
                     sw->luminanceQuantization);
            Quantize(sw->decodedYuvPixels.blocks[2], sw->decodedMcuPlanes.planes + 2 * JPEG_BLOCK_COEFFS,
                     sw->luminanceQuantization);
            Quantize(sw->decodedYuvPixels.blocks[3], sw->decodedMcuPlanes.planes + 3 * JPEG_BLOCK_COEFFS,
                     sw->luminanceQuantization);
            Quantize(sw->decodedYuvPixels.blocks[JPEG_BLOCK_CB],
                     sw->decodedMcuPlanes.planes + JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS, sw->chrominanceQuantization);
            Quantize(sw->decodedYuvPixels.blocks[JPEG_BLOCK_CR],
                     sw->decodedMcuPlanes.planes + JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS, sw->chrominanceQuantization);
            HuffmanEncode(sw->decodedYuvPixels.blocks[0], sw->luminanceDcCodes.encode,
                          sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
            HuffmanEncode(sw->decodedYuvPixels.blocks[1], sw->luminanceDcCodes.encode,
                          sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
            HuffmanEncode(sw->decodedYuvPixels.blocks[2], sw->luminanceDcCodes.encode,
                          sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
            HuffmanEncode(sw->decodedYuvPixels.blocks[3], sw->luminanceDcCodes.encode,
                          sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
            HuffmanEncode(sw->decodedYuvPixels.blocks[JPEG_BLOCK_CB], sw->chrominanceDcCodes.encode,
                          sw->chrominanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_CB]);
            HuffmanEncode(sw->decodedYuvPixels.blocks[JPEG_BLOCK_CR], sw->chrominanceDcCodes.encode,
                          sw->chrominanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_CR]);
            break;
            }
        }
    }

    if (sw->bitsRemaining != 0) {
        sw->buffer[sw->outputPosition] = sw->bufferedByte;
        sw->outputPosition++;
    }
    sw->buffer[sw->outputPosition] = JPEG_MARKER_PREFIX;
    sw->buffer[sw->outputPosition + 1] = JPEG_MARKER_EOI;
    sw->outputPosition += 2;
    request->outputSize = sw->outputPosition;
    return 0;
}

/* A DRI marker carries a big-endian restart interval in bytes 2 and 3.
 * The parser stores that value and its successor at +0x2BA0/+0x2BA4,
 * then returns the next segment byte. */
void *DefineRestartInterval(void *marker)
{
    u8 *segment = marker;
    JpegWork *work = sw;
    u32 interval;

    interval = (segment[2] << 8) + segment[3];
    work->restartInterval = interval;
    work->restartIntervalPlusOne = interval + 1;
    return segment + 4;
}

u8 *DefineQuantizeTable(u8 *marker)
{
    u8 *segment;
    u8 tableId;
    u16 count;
    float *dest;
    int index;

    segment = marker;
    count = (segment[0] << 8) + segment[1] - 2;
    segment += 2;
    do {
        tableId = segment[0];
        segment++;
        if (tableId == 0) {
            dest = sw->luminanceQuantization;
        } else {
            dest = sw->chrominanceQuantization;
        }
        for (index = 63; index >= 0; index--) {
            *dest = I2F(*segment);
            segment++;
            dest++;
        }
        count -= 65;
    } while (count != 0);
    return segment;
}

u8 *StartOfFrame(u8 *segment)
{
    JpegWork *work = sw;
    int width;

    work->imageHeight = segment[4] + (segment[3] << 8);
    width = segment[6] + (segment[5] << 8);
    work->imageWidth = width;
    work->rowBytes = (short) width * 3;
    switch (segment[9]) {
    case JPEG_SAMPLING_FACTORS_444:
        work->samplingMode = JPEG_SAMPLING_444;
        break;
    case JPEG_SAMPLING_FACTORS_422:
        work->samplingMode = JPEG_SAMPLING_422;
        break;
    case JPEG_SAMPLING_FACTORS_420:
    default:
        work->samplingMode = JPEG_SAMPLING_420;
        break;
    }
    return segment + ((segment[0] << 8) + segment[1]);
}

u8 *DefineHuffmanTable(u8 *segment)
{
    JpegWork *work;
    u8 *cursor = segment;
    u8 *end = cursor + ((cursor[0] << 8) + cursor[1]);
    u8 *counts;
    s16 *entry;
    int code;
    int length;
    int count;

    cursor += 2;
    if (cursor < end) {
        work = sw;
        do {
            switch (*cursor) {
            case JPEG_DHT_DC_LUMA:
                entry = work->luminanceDcCodes.decode;
                break;
            case JPEG_DHT_DC_CHROMA:
                entry = work->chrominanceDcCodes.decode;
                break;
            case JPEG_DHT_AC_LUMA:
                entry = work->luminanceAcCodes.decode;
                break;
            default:
                entry = work->chrominanceAcCodes.decode;
                break;
            }
            cursor++;
            code = 0;
            counts = cursor;
            cursor += JPEG_HUFF_LENGTHS;
            for (length = 0; length < JPEG_HUFF_LENGTHS; length++) {
                count = *counts++;
                while (count-- > 0) {
                    entry[0] = code;
                    code = (code + 1) & 0xFFFF;
                    entry[1] = *cursor++;
                    entry += 2;
                }
                /* The decoder recognizes 0xFE as a code-length boundary. */
                entry[1] = JPEG_HUFF_LENGTH_END;
                entry += 2;
                code = (code * 2) & 0xFFFE;
            }
            entry[1] = JPEG_HUFF_TABLE_END;
        } while (cursor < end);
    }
    return cursor;
}

/* Returns the next bit of the entropy-coded segment, MSB first. */
int GetBit(void)
{
    u8 *inputCursor;
    JpegWork *work;
    u8 value;

    if (sw->bitsRemaining == 0) {
        work = sw;
        inputCursor = work->buffer;
        value = *inputCursor;
        if (value == JPEG_MARKER_PREFIX) {
            work->buffer = inputCursor + 1;
        }
        work->bufferedByte = value;
        work->buffer++;
        sw->bitsRemaining = 7;
    } else {
        sw->bitsRemaining--;
    }
    return (sw->bufferedByte >> sw->bitsRemaining) & 1;
}

s16 decode_sub(s16 *codes)
{
    u32 code = 0;
    s16 terminator = JPEG_HUFF_LENGTH_END;

    for (;;) {
        code = code * 2 + GetBit();
        if (codes[1] != terminator) {
            do {
                if ((u16) codes[0] == code) {
                    return codes[1];
                }
                codes += 2;
            } while (codes[1] != terminator);
        }
        codes += 2;
    }
}

void HuffmanDecode(float *coefficients, int component)
{
    JpegWork *work;
    s16 *dcCodes;
    s16 *acCodes;
    float *quantization;
    int *predictor;
    int value;
    int bits;
    int dcBias;
    int acBias;
    int position;

    switch (component) {
    case JPEG_COMPONENT_Y:
    default:
        work = sw;
        dcCodes = work->luminanceDcCodes.decode;
        acCodes = work->luminanceAcCodes.decode;
        quantization = work->luminanceQuantization;
        break;
    case JPEG_COMPONENT_CB:
    case JPEG_COMPONENT_CR:
        work = sw;
        dcCodes = work->chrominanceDcCodes.decode;
        acCodes = work->chrominanceAcCodes.decode;
        quantization = work->chrominanceQuantization;
        break;
    }

    value = 0;
    predictor = &work->dcPredictors[component];
    bits = decode_sub(dcCodes);
    if (bits != 0) {
        if (GetBit() == 0) {
            dcBias = (1 << bits) - 1;
        } else {
            dcBias = 0;
            value = 1;
        }
        bits--;
        while (bits > 0) {
            bits--;
            value = value * 2 + GetBit();
        }
        value -= dcBias;
    }
    position = 1;
    value += *predictor;
    *predictor = value;
    coefficients[0] = I2F(value) * quantization[0];

    do {
        value = 0;
        bits = decode_sub(acCodes);
        if (bits != 0) {
            while (bits >= JPEG_ZRL_RUN) {
                coefficients[ZigZag[position++]] = 0.0f;
                bits -= JPEG_ZRL_RUN;
            }
            if (bits != 0) {
                if (GetBit() == 0) {
                    value = 0;
                    acBias = (1 << bits) - 1;
                } else {
                    acBias = 0;
                    value = 1;
                }
                bits--;
                while (bits > 0) {
                    bits--;
                    value = value * 2 + GetBit();
                }
                value -= acBias;
            }
            coefficients[ZigZag[position]] = I2F(value) * quantization[position];
        } else {
            while (position < JPEG_BLOCK_COEFFS) {
                coefficients[ZigZag[position++]] = 0.0f;
            }
        }
        position++;
    } while (position < JPEG_BLOCK_COEFFS);
}

/*
 * 8x8 inverse DCT on VU0. Each pass gathers one column into idctStage in
 * even/odd order, multiplies it by the basis rows already held in VF1-VF8
 * (first pass) or VF11-VF18 (second pass), and finishes the butterfly on
 * VU0 as well with VF27/VF28: the sum half goes out with sqc2, the reversed
 * difference half through qmfc2/pexew/sq. The first pass writes the rows of
 * idctBlock, the second the rows of the caller's block. XS1 did the
 * butterfly on the FPU.
 */
void FFIDCT(float block[JPEG_BLOCK_DIM][JPEG_BLOCK_DIM])
{
    float *source;
    float *column;
    float *stage;
    float *line;
    float *out;
    int i;

    stage = sw->idctStage;

    for (i = 0; i < JPEG_BLOCK_DIM; i++) {
        line = sw->idctBlock[i];
        source = block[0] + i;
        stage[0] = source[0 * JPEG_BLOCK_DIM];
        stage[1] = source[2 * JPEG_BLOCK_DIM];
        stage[2] = source[4 * JPEG_BLOCK_DIM];
        stage[3] = source[6 * JPEG_BLOCK_DIM];
        stage[4] = source[1 * JPEG_BLOCK_DIM];
        stage[5] = source[3 * JPEG_BLOCK_DIM];
        stage[6] = source[5 * JPEG_BLOCK_DIM];
        stage[7] = source[7 * JPEG_BLOCK_DIM];
        __asm__ __volatile__(
            "lqc2 vf9,0(%0)\n"
            "lqc2 vf10,16(%0)\n"
            "vmulax.xyzw ACCxyzw,vf1xyzw,vf9x\n"
            "vmadday.xyzw ACCxyzw,vf2xyzw,vf9y\n"
            "vmaddaz.xyzw ACCxyzw,vf3xyzw,vf9z\n"
            "vmaddw.xyzw vf19xyzw,vf4xyzw,vf9w\n"
            "vmulax.xyzw ACCxyzw,vf5xyzw,vf10x\n"
            "vmadday.xyzw ACCxyzw,vf6xyzw,vf10y\n"
            "vmaddaz.xyzw ACCxyzw,vf7xyzw,vf10z\n"
            "vmaddw.xyzw vf20xyzw,vf8xyzw,vf10w\n"
            "vmulax.xw ACCxw,vf27xw,vf19x\n"
            "vmulay.yz ACCyz,vf27yz,vf19y\n"
            "vmaddaz.xw ACCxw,vf28xw,vf19z\n"
            "vmaddaw.yz ACCyz,vf28yz,vf19w\n"
            "vmsub.xyzw vf10xyzw,vf27xyzw,vf20xyzw\n"
            "vmadd.xyzw vf9xyzw,vf27xyzw,vf20xyzw\n"
            "vmr32.xyzw vf19xyzw,vf10xyzw\n"
            "qmfc2.ni $2,vf19\n"
            "pexew $3,$2\n"
            "sqc2 vf9,0(%1)\n"
            "sq $3,16(%1)\n"
            :
            : "r"(stage), "r"(line)
            : "$2", "$3", "memory");
    }

    for (i = 0; i < JPEG_BLOCK_DIM; i++) {
        column = &sw->idctBlock[0][i];
        stage[0] = column[0 * JPEG_BLOCK_DIM];
        stage[1] = column[2 * JPEG_BLOCK_DIM];
        stage[2] = column[4 * JPEG_BLOCK_DIM];
        stage[3] = column[6 * JPEG_BLOCK_DIM];
        stage[4] = column[1 * JPEG_BLOCK_DIM];
        stage[5] = column[3 * JPEG_BLOCK_DIM];
        stage[6] = column[5 * JPEG_BLOCK_DIM];
        stage[7] = column[7 * JPEG_BLOCK_DIM];
        out = block[i];
        __asm__ __volatile__(
            "lqc2 vf9,0(%0)\n"
            "lqc2 vf10,16(%0)\n"
            "vmulax.xyzw ACCxyzw,vf11xyzw,vf9x\n"
            "vmadday.xyzw ACCxyzw,vf12xyzw,vf9y\n"
            "vmaddaz.xyzw ACCxyzw,vf13xyzw,vf9z\n"
            "vmaddw.xyzw vf19xyzw,vf14xyzw,vf9w\n"
            "vmulax.xyzw ACCxyzw,vf15xyzw,vf10x\n"
            "vmadday.xyzw ACCxyzw,vf16xyzw,vf10y\n"
            "vmaddaz.xyzw ACCxyzw,vf17xyzw,vf10z\n"
            "vmaddw.xyzw vf20xyzw,vf18xyzw,vf10w\n"
            "vmulax.xw ACCxw,vf27xw,vf19x\n"
            "vmulay.yz ACCyz,vf27yz,vf19y\n"
            "vmaddaz.xw ACCxw,vf28xw,vf19z\n"
            "vmaddaw.yz ACCyz,vf28yz,vf19w\n"
            "vmsub.xyzw vf10xyzw,vf27xyzw,vf20xyzw\n"
            "vmadd.xyzw vf9xyzw,vf27xyzw,vf20xyzw\n"
            "vmr32.xyzw vf19xyzw,vf10xyzw\n"
            "qmfc2.ni $2,vf19\n"
            "pexew $3,$2\n"
            "sqc2 vf9,0(%1)\n"
            "sq $3,16(%1)\n"
            :
            : "r"(stage), "r"(out)
            : "$2", "$3", "memory");
    }
}

void ConvertMCU2YUV(void)
{
    JpegWork *work = sw;
    float *planes = work->decodedMcuPlanes.planes;
    JpegYuvPixel *pixels = work->decodedYuvPixels.pixels;
    int row;
    int column;

    switch (work->samplingMode) {
    case JPEG_SAMPLING_444:
        for (row = 0; row < JPEG_BLOCK_DIM; row++) {
            for (column = 0; column < JPEG_BLOCK_DIM; column++) {
                pixels[row * JPEG_MCU_PIXELS +column].luminance = planes[row * JPEG_BLOCK_DIM + column];
                pixels[row * JPEG_MCU_PIXELS +column].firstChroma = planes[JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column];
                pixels[row * JPEG_MCU_PIXELS +column].secondChroma = planes[JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column];
            }
        }
        return;
    case JPEG_SAMPLING_422:
        for (row = 0; row < JPEG_BLOCK_DIM; row++) {
            for (column = 0; column < JPEG_BLOCK_DIM; column++) {
                pixels[row * JPEG_MCU_PIXELS +column].luminance = planes[row * JPEG_BLOCK_DIM + column];
                pixels[row * JPEG_MCU_PIXELS +column + 8].luminance = planes[JPEG_BLOCK_COEFFS + row * JPEG_BLOCK_DIM + column];
                pixels[row * JPEG_MCU_PIXELS +column * 2].firstChroma =
                    pixels[row * JPEG_MCU_PIXELS +column * 2 + 1].firstChroma =
                    planes[JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column];
                pixels[row * JPEG_MCU_PIXELS +column * 2].secondChroma =
                    pixels[row * JPEG_MCU_PIXELS +column * 2 + 1].secondChroma =
                    planes[JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column];
            }
        }
        return;
    case JPEG_SAMPLING_420:
    default:
        for (row = 0; row < JPEG_BLOCK_DIM; row++) {
            for (column = 0; column < JPEG_BLOCK_DIM; column++) {
                pixels[row * JPEG_MCU_PIXELS +column].luminance = planes[row * JPEG_BLOCK_DIM + column];
                pixels[row * JPEG_MCU_PIXELS +column + 8].luminance = planes[JPEG_BLOCK_COEFFS + row * JPEG_BLOCK_DIM + column];
                pixels[row * JPEG_MCU_PIXELS +column + 128].luminance = planes[2 * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column];
                pixels[row * JPEG_MCU_PIXELS +column + 136].luminance = planes[3 * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column];
                pixels[row * 32 + column * 2].firstChroma =
                    pixels[row * 32 + column * 2 + 1].firstChroma =
                    pixels[row * 32 + column * 2 + 16].firstChroma =
                    pixels[row * 32 + column * 2 + 17].firstChroma =
                    planes[JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column];
                pixels[row * 32 + column * 2].secondChroma =
                    pixels[row * 32 + column * 2 + 1].secondChroma =
                    pixels[row * 32 + column * 2 + 16].secondChroma =
                    pixels[row * 32 + column * 2 + 17].secondChroma =
                    planes[JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS +row * JPEG_BLOCK_DIM + column];
            }
        }
        break;
    }
}

void ConvertYUV2RGB(int width, int height)
{
    int row = 0;
    u32 *rgb = (u32 *)JPEG_STAGING_ADDRESS;

    if (height > 0) {
        do {
            JpegWork *work = sw;
            JpegYuvPixel *pixel = work->decodedYuvPixels.pixels + row * JPEG_MCU_PIXELS;

            if (width > 0) {
                int remaining = width;

                do {
                    /* StartOfScan installs VF21-VF26; FFIDCT leaves them live. */
                    __asm__ __volatile__(
                        "lqc2 vf10, 0(%0)\n\t"
                        "vmulax.xyzw ACCxyzw,vf21xyzw,vf10x\n\t"
                        "vmadday.xyzw ACCxyzw,vf22xyzw,vf10y\n\t"
                        "vmaddaz.xyzw ACCxyzw,vf23xyzw,vf10z\n\t"
                        "vmaddw.xyzw vf9xyzw,vf24xyzw,vf0w\n\t"
                        "qmfc2 $8,vf25\n\t"
                        "vftoi0.xyzw vf10xyzw,vf9xyzw\n\t"
                        "qmfc2 $9,vf26\n\t"
                        "qmfc2 $2,vf10\n\t"
                        "pmaxw $3,$2,$8\n\t"
                        "pminw $2,$3,$9\n\t"
                        "ppach $3,$0,$2\n\t"
                        "ppacb $2,$0,$3\n\t"
                        "sw $2,0(%1)"
                        :
                        : "r"(pixel), "r"(rgb)
                        : "$2", "$3", "$8", "$9", "memory");
                    remaining--;
                    pixel++;
                    rgb++;
                } while (remaining != 0);
            }
            row++;
        } while (row < height);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_jpeg", StartOfScan);

/* JPEG decode driver: sets the size limits and colour scales from the
   request, walks the marker segments of the request's buffer, dispatching
   DRI/DQT/SOF/DHT/SOS to their parsers, stops at RST0 or EOI and writes the
   decoded width and height and the parsed byte count back into the request. */
int xglJpegDecode(JpegDecodeControl *requestAddress)
{
    JpegDecodeControl *request = requestAddress;
    JpegWork *work;
    u32 extentBits;
    u8 *cursor;
    u8 marker;
    float yuvPercentScale;
    float rgbPercentScale;
    float chrominanceScale;

    /* The complete dimension pair is zero for an unbounded image. */
    __builtin_memcpy(&extentBits, &request->imageLimits, sizeof(extentBits));
    if (extentBits == 0) {
        work = sw;
        work->maximumY = work->maximumX = JPEG_NO_LIMIT;
    } else {
        work = sw;
        work->maximumX = request->imageLimits.width - 1;
        work->maximumY = request->imageLimits.height - 1;
    }
    work->restartInterval = 0;
    work->colorScaleMode = request->scaleMode;
    work->blueScale = work->greenScale = work->redScale =
        work->secondChromaScale = work->firstChromaScale =
        work->luminanceScale = 1.0f;

    switch (request->scaleMode) {
    case JPEG_SCALE_YUV:
        yuvPercentScale = JPEG_PERCENT_SCALE;
        sw->luminanceScale = I2F(request->luminancePercent) * yuvPercentScale;
        chrominanceScale = I2F(request->chrominancePercent) * yuvPercentScale;
        sw->firstChromaScale = sw->secondChromaScale = chrominanceScale;
        break;
    case JPEG_SCALE_RGB:
        rgbPercentScale = JPEG_PERCENT_SCALE;
        sw->redScale = I2F(request->redPercent) * rgbPercentScale;
        sw->greenScale = I2F(request->greenPercent) * rgbPercentScale;
        sw->blueScale = I2F(request->bluePercent) * rgbPercentScale;
        break;
    default:
        break;
    }
    cursor = request->source;
    for (;;) {
        while (*cursor++ != JPEG_MARKER_PREFIX) {
        }
        marker = *cursor;
        if (marker == JPEG_MARKER_RST0 || marker == JPEG_MARKER_EOI) {
            break;
        }
        cursor++;
        switch (marker) {
        case JPEG_MARKER_SOI:
            break;
        case JPEG_MARKER_DRI:
            cursor = DefineRestartInterval(cursor);
            break;
        case JPEG_MARKER_DQT:
            cursor = DefineQuantizeTable(cursor);
            break;
        case JPEG_MARKER_SOF0:
            cursor = StartOfFrame(cursor);
            break;
        case JPEG_MARKER_DHT:
            cursor = DefineHuffmanTable(cursor);
            break;
        case JPEG_MARKER_SOS:
            cursor = StartOfScan(cursor, requestAddress);
            break;
        default:
            cursor += (cursor[0] << 8) + cursor[1];
            break;
        }
    }
    request->imageLimits.width = sw->imageWidth;
    request->imageLimits.height = sw->imageHeight;
    request->consumedSize = cursor - request->source;
    return 0;
}
