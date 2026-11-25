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
    u8 *jpegBytes;
    u8 *decodedBuffer;
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

int xglJpegDecode(JpegDecodeControl *decodeControl);
int F2I(float value);
float I2F(int value);
extern u8 *bitstream;
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
/* The staging buffer as bytes (the RGB frame read back from the GS). */
#define JPEG_STAGING_BUFFER ((u8 *) JPEG_STAGING_ADDRESS)
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
    u16 huffCode;
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
    float planeBlocks[JPEG_MCU_BLOCKS][JPEG_BLOCK_DIM][JPEG_BLOCK_DIM];
} JpegMcuPlanes;

/* One 4 KiB buffer read as YUV pixels, as float planes, and as quantized int blocks. */
typedef union JpegDecodedYuv {
    JpegYuvPixel pixels[JPEG_MCU_PIXELS * JPEG_MCU_PIXELS];
    float floats[JPEG_MCU_PIXELS * JPEG_MCU_PIXELS * 3];
    int quantizedBlocks[16][JPEG_BLOCK_COEFFS];
} JpegDecodedYuv;

/* The codec work area is quadword aligned for FFIDCT's VU0 transfers.
 * Quantization tables occupy +0xD0 and +0x1D0; bit input, IDCT storage and
 * restart counters are modeled below. Other regions remain explicit partial
 * spans. */
typedef struct JpegWork {
    u8 gsPacketArea[0xD0];
    float luminanceQuantization[JPEG_BLOCK_COEFFS];    /* +0xD0 */
    float chrominanceQuantization[JPEG_BLOCK_COEFFS];  /* +0x1D0 */
    /* +0x2D0: the encoder's 8-bit quantizer values (luminance, chrominance),
       rebuilt from the quality and written to the DQT segments. */
    u8 quantizerValues[2][JPEG_BLOCK_COEFFS];
    u8 *bitstream;
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

static void PutBits(u16 codeBits, int length);
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
void FFDCT(float *coefficientBlock)
{
    JpegWork *codecWork = sw;
    int row;
    int column;
    int frequency;

    for (row = 0; row < JPEG_BLOCK_DIM; row++) {
        for (frequency = 0; frequency < JPEG_BLOCK_DIM; frequency++) {
            sw->idctBlock[row][frequency] =
                coefficientBlock[row * JPEG_BLOCK_DIM + 0] * dctCosineTable[frequency][0] +
                coefficientBlock[row * JPEG_BLOCK_DIM + 1] * dctCosineTable[frequency][1] +
                coefficientBlock[row * JPEG_BLOCK_DIM + 2] * dctCosineTable[frequency][2] +
                coefficientBlock[row * JPEG_BLOCK_DIM + 3] * dctCosineTable[frequency][3] +
                coefficientBlock[row * JPEG_BLOCK_DIM + 4] * dctCosineTable[frequency][4] +
                coefficientBlock[row * JPEG_BLOCK_DIM + 5] * dctCosineTable[frequency][5] +
                coefficientBlock[row * JPEG_BLOCK_DIM + 6] * dctCosineTable[frequency][6] +
                coefficientBlock[row * JPEG_BLOCK_DIM + 7] * dctCosineTable[frequency][7];
        }
    }
    for (column = 0; column < JPEG_BLOCK_DIM; column++) {
        for (frequency = 0; frequency < JPEG_BLOCK_DIM; frequency++) {
            coefficientBlock[frequency * JPEG_BLOCK_DIM + column] =
                (codecWork->idctBlock[0][column] * dctCosineTable[frequency][0] +
                codecWork->idctBlock[1][column] * dctCosineTable[frequency][1] +
                codecWork->idctBlock[2][column] * dctCosineTable[frequency][2] +
                codecWork->idctBlock[3][column] * dctCosineTable[frequency][3] +
                codecWork->idctBlock[4][column] * dctCosineTable[frequency][4] +
                codecWork->idctBlock[5][column] * dctCosineTable[frequency][5] +
                codecWork->idctBlock[6][column] * dctCosineTable[frequency][6] +
                codecWork->idctBlock[7][column] * dctCosineTable[frequency][7]) * 0.125f;
        }
    }
}

void Quantize(int *quantized, const float *coefficients, const float *reciprocals)
{
    float coefficient;
    float reciprocal;
    float scaled;
    int coefficientIndex;

    for (coefficientIndex = JPEG_BLOCK_COEFFS - 1; coefficientIndex >= 0; coefficientIndex--) {
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
    int coefficientIndex;
    int quantizer;

    if (chrominance == 0) {
        original = LQTableOrg_1;
    } else {
        original = CQTableOrg_2;
    }
    scale = quality;
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
    for (coefficientIndex = 0; coefficientIndex < JPEG_BLOCK_COEFFS; coefficientIndex++) {
        quantizer = F2I((original[coefficientIndex] * scale + JPEG_QUANT_ROUNDING) * JPEG_PERCENT_SCALE);
        if (quantizer <= 0) {
            quantizer = 1;
        }
        if (quantizer >= JPEG_QUANT_MAX + 1) {
            quantizer = JPEG_QUANT_MAX;
        }
        quantizers[coefficientIndex] = quantizer;
        *reciprocalDest++ = 1.0f / I2F(quantizer);
    }
}

static void PutBits(u16 codeBits, int length)
{
    u8 *bitstreamBytes = sw->bitstream;
    int bits = codeBits;
    int bufferedBits;
    JpegWork *codecWork;

    if (length != 0) {
        bits &= (1 << length) - 1;
        while (length + sw->bitsRemaining >= 8) {
            sw->bufferedByte |= bits >> (length + sw->bitsRemaining - 8);
            bitstreamBytes[sw->outputPosition] = sw->bufferedByte;
            if (sw->bufferedByte == JPEG_MARKER_PREFIX) {
                sw->outputPosition++;
                bitstreamBytes[sw->outputPosition] = 0;
            }
            codecWork = sw;
            bufferedBits = codecWork->bitsRemaining;
            codecWork->bitsRemaining = 0;
            length = length + bufferedBits - 8;
            codecWork->outputPosition++;
            sw->bufferedByte = 0;
        }
        if (length > 0) {
            sw->bufferedByte |= bits << (8 - sw->bitsRemaining - length);
            sw->bitsRemaining += length;
        }
    }
}

static void EncodeDc(int dcDifference, JpegHuffCode *huffTable)
{
    int magnitude;
    int negative;
    int remaining;
    int length;
    u16 bits;

    magnitude = dcDifference;
    if (magnitude == 0) {
        PutBits(huffTable[0].huffCode, huffTable[0].length);
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
    PutBits(huffTable[length].huffCode, huffTable[length].length);
    if (negative) {
        bits = ~magnitude & 0xFFFF;
    } else {
        bits = magnitude & 0xFFFF;
    }
    PutBits(bits, length);
}

static void EncodeAc(int acCoefficient, int runSymbolIndex, JpegHuffCode *huffTable)
{
    int magnitude;
    int negative;
    int remaining;
    int length;
    u16 bits;

    magnitude = acCoefficient;
    if (magnitude == 0) {
        PutBits(huffTable[runSymbolIndex].huffCode, huffTable[runSymbolIndex].length);
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

    PutBits(huffTable[length + runSymbolIndex].huffCode, huffTable[length + runSymbolIndex].length);

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
    short acCoefficient;
    short zeroRunLength;
    int zigzagIndex;

    zeroRunLength = 0;
    zigzagIndex = 1;
    difference = coefficients[0] - *previousDc;
    *previousDc += difference;
    EncodeDc(difference, dcCodes);

    for (;;) {
        acCoefficient = coefficients[ZigZag[zigzagIndex]];
        zigzagIndex++;
        if (acCoefficient == 0) {
            if (zigzagIndex >= JPEG_BLOCK_COEFFS) {
                EncodeAc(0, 0, acCodes);
                break;
            }
            zeroRunLength++;
        } else {
            while (zeroRunLength >= JPEG_ZRL_RUN) {
                EncodeAc(0, JPEG_AC_ZRL, acCodes);
                zeroRunLength -= JPEG_ZRL_RUN;
            }
            EncodeAc(acCoefficient, zeroRunLength * 16, acCodes);
            if (zigzagIndex >= JPEG_BLOCK_COEFFS) {
                break;
            }
            zeroRunLength = 0;
        }
    }
}

void ExtractHuffmanTableSub(JpegHuffCode *huffTable, const u8 *symbols, int symbolCount)
{
    const u8 *codeLengthCounts;
    int nextHuffmanCode;
    s16 length;
    int lengthsRemaining;
    int symbolsRemaining;
    JpegHuffCode *huffEntry;

    memset(huffTable, 0, symbolCount * 4);
    codeLengthCounts = symbols;
    nextHuffmanCode = 0;
    length = 1;
    symbols += JPEG_HUFF_LENGTHS;
    for (lengthsRemaining = JPEG_HUFF_LENGTHS - 1; lengthsRemaining >= 0; lengthsRemaining--) {
        symbolsRemaining = *codeLengthCounts++;
        while (symbolsRemaining-- > 0) {
            huffEntry = &huffTable[*symbols++];
            huffEntry->huffCode = nextHuffmanCode;
            nextHuffmanCode = (nextHuffmanCode + 1) & 0xFFFF;
            huffEntry->length = length;
        }
        nextHuffmanCode = (nextHuffmanCode * 2) & 0xFFFE;
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

int xglJpegEncode(JpegEncodeRequest *encodeRequest)
{
    u8 *headerPatchCursor;
    u8 *outputBuffer;
    int mcuY;
    int mcuX;
    int row;
    int column;
    int index;
    int mcuWidth;
    int mcuHeight;
    int densityValue;

    sw->rowBytes = encodeRequest->width * 3;
    RebuildQuantizeTable(sw->quantizerValues[0], sw->luminanceQuantization, 0, encodeRequest->luminanceQuality);
    RebuildQuantizeTable(sw->quantizerValues[1], sw->chrominanceQuantization, 1,
                         encodeRequest->chrominanceQuality);
    ExtractHuffmanTableSub(sw->luminanceDcCodes.encode, headerALL + JPEG_HEADER_DHT_DC_LUMA_OFFSET,
                           JPEG_HUFF_DC_SYMBOLS);
    ExtractHuffmanTableSub(sw->chrominanceDcCodes.encode, headerALL + JPEG_HEADER_DHT_DC_CHROMA_OFFSET,
                           JPEG_HUFF_DC_SYMBOLS);
    ExtractHuffmanTableSub(sw->luminanceAcCodes.encode, headerALL + JPEG_HEADER_DHT_AC_LUMA_OFFSET,
                           JPEG_HUFF_AC_SYMBOLS);
    ExtractHuffmanTableSub(sw->chrominanceAcCodes.encode, headerALL + JPEG_HEADER_DHT_AC_CHROMA_OFFSET,
                           JPEG_HUFF_AC_SYMBOLS);

    sw->bitstream = encodeRequest->outputBuffer;
    sw->bitsRemaining = 0;
    sw->bufferedByte = 0;
    __builtin_memcpy(sw->bitstream, headerALL, JPEG_HEADER_SIZE);

    sw->outputPosition = JPEG_HEADER_SIZE;
    headerPatchCursor = sw->bitstream + JPEG_HEADER_DENSITY_OFFSET;
    densityValue = JPEG_DENSITY_HEIGHT_NUMERATOR / encodeRequest->height;
    headerPatchCursor[1] = densityValue;
    headerPatchCursor[0] = densityValue >> 8;
    densityValue = JPEG_DENSITY_WIDTH_NUMERATOR / encodeRequest->width;
    headerPatchCursor[3] = densityValue;
    headerPatchCursor[2] = densityValue >> 8;

    headerPatchCursor = sw->bitstream + JPEG_HEADER_DQT_LUMA_OFFSET;
    for (index = 0; index < JPEG_BLOCK_COEFFS; index++) {
        *headerPatchCursor = sw->quantizerValues[0][ZigZag[index]];
        headerPatchCursor++;
    }
    headerPatchCursor = sw->bitstream + JPEG_HEADER_DQT_CHROMA_OFFSET;
    for (index = 0; index < JPEG_BLOCK_COEFFS; index++) {
        *headerPatchCursor = sw->quantizerValues[1][ZigZag[index]];
        headerPatchCursor++;
    }

    switch (encodeRequest->samplingMode) {
    case JPEG_SAMPLING_444:
        index = JPEG_SAMPLING_FACTORS_444;
        break;
    case JPEG_SAMPLING_422:
        index = JPEG_SAMPLING_FACTORS_422;
        break;
    case JPEG_SAMPLING_420:
    default:
        index = JPEG_SAMPLING_FACTORS_420;
        break;
    }
    headerPatchCursor = sw->bitstream + JPEG_HEADER_SOF_SIZE_OFFSET;
    headerPatchCursor[0] = encodeRequest->height >> 8;
    headerPatchCursor[1] = encodeRequest->height;
    headerPatchCursor[2] = encodeRequest->width >> 8;
    headerPatchCursor[3] = encodeRequest->width;
    headerPatchCursor[6] = index;

    sw->dcPredictors[JPEG_COMPONENT_CR] = 0;
    sw->dcPredictors[JPEG_COMPONENT_CB] = 0;
    sw->dcPredictors[JPEG_COMPONENT_Y] = 0;

    switch (encodeRequest->samplingMode) {
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

    for (mcuY = 0; mcuY < encodeRequest->height; mcuY += mcuHeight) {
        for (mcuX = 0; mcuX < encodeRequest->width; mcuX += mcuWidth) {
            void *storeImage = &sw->gsPacketArea[JPEG_STORE_IMAGE_OFFSET];

            sceGsSetDefStoreImage(storeImage, encodeRequest->frameBufferBase, encodeRequest->frameBufferWidth,
                                  GS_PSMCT24, encodeRequest->originX + mcuX, encodeRequest->originY + mcuY,
                                  /* Zero-extended with sll/srl as in the original; a (u16) cast
                                     compiles to andi and moves the argument loads. */
                                  ((unsigned) mcuWidth << 16) >> 16, ((unsigned) mcuHeight << 16) >> 16);
            sceGsExecStoreImage(&sw->gsPacketArea[JPEG_STORE_IMAGE_OFFSET], JPEG_STAGING_BUFFER);

            for (row = 0; row < mcuHeight; row++) {
                u8 *rgb = JPEG_STAGING_BUFFER + row * mcuWidth * 3;
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

            ConvertYUV2MCU(sw->decodedMcuPlanes.planes, sw->decodedYuvPixels.floats, encodeRequest->samplingMode);

            switch (encodeRequest->samplingMode) {
            case JPEG_SAMPLING_444:
                FFDCT(sw->decodedMcuPlanes.planes + 0);
                FFDCT(sw->decodedMcuPlanes.planes + JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS);
                FFDCT(sw->decodedMcuPlanes.planes + JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS);
                Quantize(sw->decodedYuvPixels.quantizedBlocks[0], sw->decodedMcuPlanes.planes + 0, sw->luminanceQuantization);
                Quantize(sw->decodedYuvPixels.quantizedBlocks[JPEG_BLOCK_CB],
                         sw->decodedMcuPlanes.planes + JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS, sw->chrominanceQuantization);
                Quantize(sw->decodedYuvPixels.quantizedBlocks[JPEG_BLOCK_CR],
                         sw->decodedMcuPlanes.planes + JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS, sw->chrominanceQuantization);
                HuffmanEncode(sw->decodedYuvPixels.quantizedBlocks[0], sw->luminanceDcCodes.encode,
                              sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
                HuffmanEncode(sw->decodedYuvPixels.quantizedBlocks[JPEG_BLOCK_CB], sw->chrominanceDcCodes.encode,
                              sw->chrominanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_CB]);
                HuffmanEncode(sw->decodedYuvPixels.quantizedBlocks[JPEG_BLOCK_CR], sw->chrominanceDcCodes.encode,
                              sw->chrominanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_CR]);
                break;
            case JPEG_SAMPLING_422:
                FFDCT(sw->decodedMcuPlanes.planes + 0);
                FFDCT(sw->decodedMcuPlanes.planes + JPEG_BLOCK_COEFFS);
                FFDCT(sw->decodedMcuPlanes.planes + JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS);
                FFDCT(sw->decodedMcuPlanes.planes + JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS);
                Quantize(sw->decodedYuvPixels.quantizedBlocks[0], sw->decodedMcuPlanes.planes + 0, sw->luminanceQuantization);
                Quantize(sw->decodedYuvPixels.quantizedBlocks[1], sw->decodedMcuPlanes.planes + JPEG_BLOCK_COEFFS,
                         sw->luminanceQuantization);
                Quantize(sw->decodedYuvPixels.quantizedBlocks[JPEG_BLOCK_CB],
                         sw->decodedMcuPlanes.planes + JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS, sw->chrominanceQuantization);
                Quantize(sw->decodedYuvPixels.quantizedBlocks[JPEG_BLOCK_CR],
                         sw->decodedMcuPlanes.planes + JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS, sw->chrominanceQuantization);
                HuffmanEncode(sw->decodedYuvPixels.quantizedBlocks[0], sw->luminanceDcCodes.encode,
                              sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
                HuffmanEncode(sw->decodedYuvPixels.quantizedBlocks[1], sw->luminanceDcCodes.encode,
                              sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
                HuffmanEncode(sw->decodedYuvPixels.quantizedBlocks[JPEG_BLOCK_CB], sw->chrominanceDcCodes.encode,
                              sw->chrominanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_CB]);
                HuffmanEncode(sw->decodedYuvPixels.quantizedBlocks[JPEG_BLOCK_CR], sw->chrominanceDcCodes.encode,
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
                Quantize(sw->decodedYuvPixels.quantizedBlocks[0], sw->decodedMcuPlanes.planes + 0, sw->luminanceQuantization);
                Quantize(sw->decodedYuvPixels.quantizedBlocks[1], sw->decodedMcuPlanes.planes + JPEG_BLOCK_COEFFS,
                         sw->luminanceQuantization);
                Quantize(sw->decodedYuvPixels.quantizedBlocks[2], sw->decodedMcuPlanes.planes + 2 * JPEG_BLOCK_COEFFS,
                         sw->luminanceQuantization);
                Quantize(sw->decodedYuvPixels.quantizedBlocks[3], sw->decodedMcuPlanes.planes + 3 * JPEG_BLOCK_COEFFS,
                         sw->luminanceQuantization);
                Quantize(sw->decodedYuvPixels.quantizedBlocks[JPEG_BLOCK_CB],
                         sw->decodedMcuPlanes.planes + JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS, sw->chrominanceQuantization);
                Quantize(sw->decodedYuvPixels.quantizedBlocks[JPEG_BLOCK_CR],
                         sw->decodedMcuPlanes.planes + JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS, sw->chrominanceQuantization);
                HuffmanEncode(sw->decodedYuvPixels.quantizedBlocks[0], sw->luminanceDcCodes.encode,
                              sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
                HuffmanEncode(sw->decodedYuvPixels.quantizedBlocks[1], sw->luminanceDcCodes.encode,
                              sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
                HuffmanEncode(sw->decodedYuvPixels.quantizedBlocks[2], sw->luminanceDcCodes.encode,
                              sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
                HuffmanEncode(sw->decodedYuvPixels.quantizedBlocks[3], sw->luminanceDcCodes.encode,
                              sw->luminanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_Y]);
                HuffmanEncode(sw->decodedYuvPixels.quantizedBlocks[JPEG_BLOCK_CB], sw->chrominanceDcCodes.encode,
                              sw->chrominanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_CB]);
                HuffmanEncode(sw->decodedYuvPixels.quantizedBlocks[JPEG_BLOCK_CR], sw->chrominanceDcCodes.encode,
                              sw->chrominanceAcCodes.encode, &sw->dcPredictors[JPEG_COMPONENT_CR]);
                break;
            }
        }
    }

    if (sw->bitsRemaining != 0) {
        sw->bitstream[sw->outputPosition] = sw->bufferedByte;
        sw->outputPosition++;
    }
    sw->bitstream[sw->outputPosition] = JPEG_MARKER_PREFIX;
    sw->bitstream[sw->outputPosition + 1] = JPEG_MARKER_EOI;
    sw->outputPosition += 2;
    encodeRequest->outputSize = sw->outputPosition;
    return 0;
}

/* A DRI marker carries a big-endian restart interval in bytes 2 and 3.
 * The parser stores that value and its successor at +0x2BA0/+0x2BA4,
 * then returns the next segment byte. */
u8 *DefineRestartInterval(u8 *marker)
{
    u8 *segmentCursor = marker;
    JpegWork *codecWork = sw;
    u32 interval;

    interval = (segmentCursor[2] << 8) + segmentCursor[3];
    codecWork->restartInterval = interval;
    codecWork->restartIntervalPlusOne = interval + 1;
    return segmentCursor + 4;
}

u8 *DefineQuantizeTable(u8 *marker)
{
    u8 *segmentCursor;
    u8 tableId;
    u16 segmentBytesRemaining;
    float *quantTable;
    int coefficientIndex;

    segmentCursor = marker;
    segmentBytesRemaining = (segmentCursor[0] << 8) + segmentCursor[1] - 2;
    segmentCursor += 2;
    do {
        tableId = segmentCursor[0];
        segmentCursor++;
        if (tableId == 0) {
            quantTable = sw->luminanceQuantization;
        } else {
            quantTable = sw->chrominanceQuantization;
        }
        for (coefficientIndex = 63; coefficientIndex >= 0; coefficientIndex--) {
            *quantTable = I2F(*segmentCursor);
            segmentCursor++;
            quantTable++;
        }
        segmentBytesRemaining -= 65;
    } while (segmentBytesRemaining != 0);
    return segmentCursor;
}

u8 *StartOfFrame(u8 *frameSegment)
{
    JpegWork *codecWork = sw;
    int width;

    codecWork->imageHeight = frameSegment[4] + (frameSegment[3] << 8);
    width = frameSegment[6] + (frameSegment[5] << 8);
    codecWork->imageWidth = width;
    codecWork->rowBytes = (short) width * 3;
    switch (frameSegment[9]) {
    case JPEG_SAMPLING_FACTORS_444:
        codecWork->samplingMode = JPEG_SAMPLING_444;
        break;
    case JPEG_SAMPLING_FACTORS_422:
        codecWork->samplingMode = JPEG_SAMPLING_422;
        break;
    case JPEG_SAMPLING_FACTORS_420:
    default:
        codecWork->samplingMode = JPEG_SAMPLING_420;
        break;
    }
    return frameSegment + ((frameSegment[0] << 8) + frameSegment[1]);
}

u8 *DefineHuffmanTable(u8 *dhtSegment)
{
    JpegWork *codecWork;
    u8 *cursor = dhtSegment;
    u8 *dhtSegmentEnd = cursor + ((cursor[0] << 8) + cursor[1]);
    u8 *codeLengthCounts;
    s16 *decodeEntry;
    int huffmanCode;
    int length;
    int codesOfLength;

    cursor += 2;
    if (cursor < dhtSegmentEnd) {
        codecWork = sw;
        do {
            switch (*cursor) {
            case JPEG_DHT_DC_LUMA:
                decodeEntry = codecWork->luminanceDcCodes.decode;
                break;
            case JPEG_DHT_DC_CHROMA:
                decodeEntry = codecWork->chrominanceDcCodes.decode;
                break;
            case JPEG_DHT_AC_LUMA:
                decodeEntry = codecWork->luminanceAcCodes.decode;
                break;
            default:
                decodeEntry = codecWork->chrominanceAcCodes.decode;
                break;
            }
            cursor++;
            huffmanCode = 0;
            codeLengthCounts = cursor;
            cursor += JPEG_HUFF_LENGTHS;
            for (length = 0; length < JPEG_HUFF_LENGTHS; length++) {
                codesOfLength = *codeLengthCounts++;
                while (codesOfLength-- > 0) {
                    decodeEntry[0] = huffmanCode;
                    huffmanCode = (huffmanCode + 1) & 0xFFFF;
                    decodeEntry[1] = *cursor++;
                    decodeEntry += 2;
                }
                /* The decoder recognizes 0xFE as a code-length boundary. */
                decodeEntry[1] = JPEG_HUFF_LENGTH_END;
                decodeEntry += 2;
                huffmanCode = (huffmanCode * 2) & 0xFFFE;
            }
            decodeEntry[1] = JPEG_HUFF_TABLE_END;
        } while (cursor < dhtSegmentEnd);
    }
    return cursor;
}

/* Returns the next bit of the entropy-coded segment, MSB first. */
int GetBit(void)
{
    u8 *inputCursor;
    JpegWork *codecWork;
    u8 entropyByte;

    if (sw->bitsRemaining == 0) {
        codecWork = sw;
        inputCursor = codecWork->bitstream;
        entropyByte = *inputCursor;
        if (entropyByte == JPEG_MARKER_PREFIX) {
            codecWork->bitstream = inputCursor + 1;
        }
        codecWork->bufferedByte = entropyByte;
        codecWork->bitstream++;
        sw->bitsRemaining = 7;
    } else {
        sw->bitsRemaining--;
    }
    return (sw->bufferedByte >> sw->bitsRemaining) & 1;
}

s16 decode_sub(s16 *decodeTable)
{
    u32 prefixCode = 0;
    s16 terminator = JPEG_HUFF_LENGTH_END;

    for (;;) {
        prefixCode = prefixCode * 2 + GetBit();
        if (decodeTable[1] != terminator) {
            do {
                if ((u16) decodeTable[0] == prefixCode) {
                    return decodeTable[1];
                }
                decodeTable += 2;
            } while (decodeTable[1] != terminator);
        }
        decodeTable += 2;
    }
}

void HuffmanDecode(float *coefficients, int component)
{
    JpegWork *codecWork;
    s16 *dcCodes;
    s16 *acCodes;
    float *quantization;
    int *predictor;
    int coefficientValue;
    int bits;
    int dcBias;
    int acBias;
    int position;

    switch (component) {
    case JPEG_COMPONENT_Y:
    default:
        codecWork = sw;
        dcCodes = codecWork->luminanceDcCodes.decode;
        acCodes = codecWork->luminanceAcCodes.decode;
        quantization = codecWork->luminanceQuantization;
        break;
    case JPEG_COMPONENT_CB:
    case JPEG_COMPONENT_CR:
        codecWork = sw;
        dcCodes = codecWork->chrominanceDcCodes.decode;
        acCodes = codecWork->chrominanceAcCodes.decode;
        quantization = codecWork->chrominanceQuantization;
        break;
    }

    coefficientValue = 0;
    predictor = &codecWork->dcPredictors[component];
    bits = decode_sub(dcCodes);
    if (bits != 0) {
        if (GetBit() == 0) {
            dcBias = (1 << bits) - 1;
        } else {
            dcBias = 0;
            coefficientValue = 1;
        }
        bits--;
        while (bits > 0) {
            bits--;
            coefficientValue = coefficientValue * 2 + GetBit();
        }
        coefficientValue -= dcBias;
    }
    position = 1;
    coefficientValue += *predictor;
    *predictor = coefficientValue;
    coefficients[0] = I2F(coefficientValue) * quantization[0];

    do {
        coefficientValue = 0;
        bits = decode_sub(acCodes);
        if (bits != 0) {
            while (bits >= JPEG_ZRL_RUN) {
                coefficients[ZigZag[position++]] = 0.0f;
                bits -= JPEG_ZRL_RUN;
            }
            if (bits != 0) {
                if (GetBit() == 0) {
                    coefficientValue = 0;
                    acBias = (1 << bits) - 1;
                } else {
                    acBias = 0;
                    coefficientValue = 1;
                }
                bits--;
                while (bits > 0) {
                    bits--;
                    coefficientValue = coefficientValue * 2 + GetBit();
                }
                coefficientValue -= acBias;
            }
            coefficients[ZigZag[position]] = I2F(coefficientValue) * quantization[position];
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
void FFIDCT(float coefficientBlock[JPEG_BLOCK_DIM][JPEG_BLOCK_DIM])
{
    float *blockColumn;
    float *idctColumn;
    float *stage;
    float *idctRow;
    float *outputRow;
    int index;

    stage = sw->idctStage;

    for (index = 0; index < JPEG_BLOCK_DIM; index++) {
        idctRow = sw->idctBlock[index];
        blockColumn = coefficientBlock[0] + index;
        stage[0] = blockColumn[0 * JPEG_BLOCK_DIM];
        stage[1] = blockColumn[2 * JPEG_BLOCK_DIM];
        stage[2] = blockColumn[4 * JPEG_BLOCK_DIM];
        stage[3] = blockColumn[6 * JPEG_BLOCK_DIM];
        stage[4] = blockColumn[1 * JPEG_BLOCK_DIM];
        stage[5] = blockColumn[3 * JPEG_BLOCK_DIM];
        stage[6] = blockColumn[5 * JPEG_BLOCK_DIM];
        stage[7] = blockColumn[7 * JPEG_BLOCK_DIM];
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
            : "r"(stage), "r"(idctRow)
            : "$2", "$3", "memory");
    }

    for (index = 0; index < JPEG_BLOCK_DIM; index++) {
        idctColumn = &sw->idctBlock[0][index];
        stage[0] = idctColumn[0 * JPEG_BLOCK_DIM];
        stage[1] = idctColumn[2 * JPEG_BLOCK_DIM];
        stage[2] = idctColumn[4 * JPEG_BLOCK_DIM];
        stage[3] = idctColumn[6 * JPEG_BLOCK_DIM];
        stage[4] = idctColumn[1 * JPEG_BLOCK_DIM];
        stage[5] = idctColumn[3 * JPEG_BLOCK_DIM];
        stage[6] = idctColumn[5 * JPEG_BLOCK_DIM];
        stage[7] = idctColumn[7 * JPEG_BLOCK_DIM];
        outputRow = coefficientBlock[index];
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
            : "r"(stage), "r"(outputRow)
            : "$2", "$3", "memory");
    }
}

void ConvertMCU2YUV(void)
{
    JpegWork *codecWork = sw;
    float *planes = codecWork->decodedMcuPlanes.planes;
    JpegYuvPixel *pixels = codecWork->decodedYuvPixels.pixels;
    int row;
    int column;

    switch (codecWork->samplingMode) {
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
    u32 *texel = (u32 *)JPEG_STAGING_ADDRESS;

    if (height > 0) {
        do {
            JpegWork *codecWork = sw;
            JpegYuvPixel *pixel = codecWork->decodedYuvPixels.pixels + row * JPEG_MCU_PIXELS;

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
                        : "r"(pixel), "r"(texel)
                        : "$2", "$3", "$8", "$9", "memory");
                    remaining--;
                    pixel++;
                    texel++;
                } while (remaining != 0);
            }
            row++;
        } while (row < height);
    }
}

u8 *StartOfScan(u8 *segment, JpegDecodeControl *requestAddress)
{
    JpegDecodeControl *request = requestAddress;
    int mcuWidth;
    int mcuHeight;
    int mcuX;
    int mcuY;
    int texelRow;
    u8 *destCursor;
    u8 *stagedRgba;

    /* VU0 setup: the I register is loaded with 0.125f through $2 (hence the clobber),
     * vf1-vf8 and vf27/vf28 are loaded, vf1-vf8 are scaled into vf11-vf18, then vf21-vf26 are loaded. */
    __asm__ __volatile__(
        "lui $2,0x3e00\n\t"
        "ctc2 $2,$vi21\n\t"
        "lqc2 vf1,0(%0)\n\t"
        "lqc2 vf2,16(%0)\n\t"
        "lqc2 vf3,32(%0)\n\t"
        "lqc2 vf4,48(%0)\n\t"
        "lqc2 vf5,64(%0)\n\t"
        "lqc2 vf6,80(%0)\n\t"
        "lqc2 vf7,96(%0)\n\t"
        "lqc2 vf8,112(%0)\n\t"
        "lqc2 vf27,128(%0)\n\t"
        "lqc2 vf28,144(%0)\n\t"
        "vmuli.xyzw vf11xyzw,vf1xyzw,I\n\t"
        "vmuli.xyzw vf12xyzw,vf2xyzw,I\n\t"
        "vmuli.xyzw vf13xyzw,vf3xyzw,I\n\t"
        "vmuli.xyzw vf14xyzw,vf4xyzw,I\n\t"
        "vmuli.xyzw vf15xyzw,vf5xyzw,I\n\t"
        "vmuli.xyzw vf16xyzw,vf6xyzw,I\n\t"
        "vmuli.xyzw vf17xyzw,vf7xyzw,I\n\t"
        "vmuli.xyzw vf18xyzw,vf8xyzw,I\n\t"
        :
        : "r"(imm0_3)
        : "$2", "memory");
    __asm__ __volatile__(
        "lqc2 vf21,0(%0)\n\t"
        "lqc2 vf22,16(%0)\n\t"
        "lqc2 vf23,32(%0)\n\t"
        "lqc2 vf24,48(%0)\n\t"
        "lqc2 vf25,64(%0)\n\t"
        "lqc2 vf26,80(%0)\n\t"
        :
        : "r"(imm1_4)
        : "memory");

    /* Skip the scan header (length, component count, selectors, spectral range). */
    segment += 12;
    sw->bitstream = segment; /* cursor into the entropy-coded data */
    sw->bitsRemaining = 0; /* bit reader starts byte-aligned */
    sw->bufferedByte = 0; /* nothing buffered from the stream yet */
    sw->dcPredictors[JPEG_COMPONENT_CR] = 0;
    sw->dcPredictors[JPEG_COMPONENT_CB] = 0;
    sw->dcPredictors[JPEG_COMPONENT_Y] = 0;

    /* MCU size: 8x8 (mode 0), 16x8 (mode 1) or 16x16 (mode 2). */
    switch (sw->samplingMode) {
    case JPEG_SAMPLING_444:
        mcuWidth = JPEG_BLOCK_DIM;
        mcuHeight = JPEG_BLOCK_DIM;
        break;
    case JPEG_SAMPLING_422:
        mcuWidth = JPEG_MCU_PIXELS;
        mcuHeight = JPEG_BLOCK_DIM;
        break;
    case JPEG_SAMPLING_420:
    default:
        mcuWidth = JPEG_MCU_PIXELS;
        mcuHeight = JPEG_MCU_PIXELS;
        break;
    }

    /* Decode the image one MCU at a time, left to right, top to bottom. */
    for (mcuY = 0; mcuY < sw->imageHeight; mcuY += mcuHeight) {
        for (mcuX = 0; mcuX < sw->imageWidth; mcuX += mcuWidth) {
            u32 restartPeriod = sw->restartInterval;
            /* A restart interval resets the bit reader and the DC predictors. */
            if (restartPeriod != 0 && --sw->restartIntervalPlusOne == 0) {
                sw->restartIntervalPlusOne = restartPeriod; /* restart the interval countdown */
                sw->bitstream += 2; /* step over the RSTn marker */
                sw->bitsRemaining = 0; /* no bits left in the current byte */
                sw->dcPredictors[JPEG_COMPONENT_CR] = 0;
                sw->dcPredictors[JPEG_COMPONENT_CB] = 0;
                sw->dcPredictors[JPEG_COMPONENT_Y] = 0;
            }

            /* Block order: component 0 (1, 2 or 4 blocks), then components 1 and 2. */
            HuffmanDecode(sw->decodedMcuPlanes.planes, JPEG_COMPONENT_Y);
            if (sw->samplingMode > 0) {
                HuffmanDecode(sw->decodedMcuPlanes.planes + JPEG_BLOCK_COEFFS, JPEG_COMPONENT_Y);
            }
            if (sw->samplingMode == JPEG_SAMPLING_420) {
                HuffmanDecode(sw->decodedMcuPlanes.planes + 2 * JPEG_BLOCK_COEFFS, JPEG_COMPONENT_Y);
                HuffmanDecode(sw->decodedMcuPlanes.planes + 3 * JPEG_BLOCK_COEFFS, JPEG_COMPONENT_Y);
            }
            HuffmanDecode(sw->decodedMcuPlanes.planes + JPEG_BLOCK_CB * JPEG_BLOCK_COEFFS, JPEG_COMPONENT_CB);
            HuffmanDecode(sw->decodedMcuPlanes.planes + JPEG_BLOCK_CR * JPEG_BLOCK_COEFFS, JPEG_COMPONENT_CR);

            /* MCUs outside the requested crop window are decoded but not converted or stored. */
            if (mcuX <= sw->maximumX && mcuY <= sw->maximumY) {
                /* Inverse DCT of each decoded 8x8 block, in decode order. */
                FFIDCT(sw->decodedMcuPlanes.planeBlocks[0]);
                if (sw->samplingMode > 0) {
                    FFIDCT(sw->decodedMcuPlanes.planeBlocks[1]);
                }
                if (sw->samplingMode == JPEG_SAMPLING_420) {
                    FFIDCT(sw->decodedMcuPlanes.planeBlocks[2]);
                    FFIDCT(sw->decodedMcuPlanes.planeBlocks[3]);
                }
                FFIDCT(sw->decodedMcuPlanes.planeBlocks[JPEG_BLOCK_CB]);
                FFIDCT(sw->decodedMcuPlanes.planeBlocks[JPEG_BLOCK_CR]);
                /* Gather the IDCT output into per-pixel YUV, then convert to RGBA at 0x70003000. */
                ConvertMCU2YUV();
                ConvertYUV2RGB(mcuWidth, mcuHeight);

                /* No destination buffer: upload the MCU to the GS (frame buffer base and width come from the
                 * request). */
                if (request->decodedBuffer == 0) {
                    /* The load-image packet lives in the work area; the MCU is read from 0x70003000. */
                    sceGsSetDefLoadImage(sw, request->frameBufferBase, request->frameBufferWidth, GS_PSMCT32,
                                         (short)(request->originX + mcuX), /* GS x */
                                         (short)(request->originY + mcuY), /* GS y */
                                         (short)mcuWidth, (short)mcuHeight); /* upload size: one MCU */
                    sceGsExecLoadImage(sw, (void *)JPEG_STAGING_ADDRESS);
                } else if (request->pixelFormat & JPEG_PIXEL_FORMAT_RGB555) {
                    /* 16-bit RGB555 destination: pack each RGBA texel. */
                    for (texelRow = 0; texelRow < mcuHeight; texelRow++) {
                        int texelsLeft;

                        stagedRgba = (u8 *)JPEG_STAGING_ADDRESS + texelRow * (mcuWidth * JPEG_RGBA_BYTES);
                        /* destCursor stays a byte pointer shared with the 32-bit branch:
                           the original keeps both cursors in $17, and a separate u16 *
                           local moves this one to $7 and renumbers the loop registers.
                           The fields are added, not or-ed: with | the pack is scheduled
                           differently. */
                        destCursor = request->decodedBuffer
                                     + ((mcuY + texelRow) * sw->imageWidth + mcuX) * JPEG_RGB555_BYTES;
                        texelsLeft = sw->imageWidth < mcuX + mcuWidth ? sw->imageWidth - mcuX : mcuWidth;
                        for (; texelsLeft > 0; texelsLeft--) {
                            int green555 = (stagedRgba[1] >> 3) << 5;
                            int blue555 = (stagedRgba[2] >> 3) << 10;

                            *(u16 *)destCursor = (u16)((stagedRgba[0] >> 3) + (blue555 + green555));
                            destCursor += JPEG_RGB555_BYTES; /* advance one RGB555 pixel */
                            stagedRgba += JPEG_RGBA_BYTES; /* advance one RGBA texel */
                        }
                    }
                } else {
                    /* 32-bit destination: copy each MCU row. */
                    destCursor = request->decodedBuffer + (mcuY * sw->imageWidth + mcuX) * JPEG_RGBA_BYTES;
                    stagedRgba = (u8 *)JPEG_STAGING_ADDRESS;

                    for (texelRow = 0; texelRow < mcuHeight; texelRow++) {
                        if (sw->imageWidth < mcuX + mcuWidth) {
                            /* Clip the last MCU to the image width. */
                            memcpy(destCursor, stagedRgba, (sw->imageWidth - mcuX) * JPEG_RGBA_BYTES);
                        } else {
                            memcpy(destCursor, stagedRgba, mcuWidth * JPEG_RGBA_BYTES);
                        }
                        destCursor += sw->imageWidth * JPEG_RGBA_BYTES; /* advance one destination row */
                        stagedRgba += mcuWidth * JPEG_RGBA_BYTES; /* advance one staged row */
                    }
                }
            }
        }
    }

    /* Scan for the next marker: a 0xFF that is not a stuffed 0x00 byte. The
     * returned pointer addresses that 0xFF. */
    for (;;) {
        while (*segment++ != JPEG_MARKER_PREFIX) {
        }
        if (*segment != 0) {
            return segment - 1; /* return the address of the marker byte */
        }
    }
}

/* JPEG decode driver: sets the size limits and colour scales from the
   request, walks the marker segments of the request's buffer, dispatching
   DRI/DQT/SOF/DHT/SOS to their parsers, stops at RST0 or EOI and writes the
   decoded width and height and the parsed byte count back into the request. */
int xglJpegDecode(JpegDecodeControl *decodeControl)
{
    JpegWork *codecWork;
    u32 extentBits;
    u8 *cursor;
    u8 marker;
    float yuvPercentScale;
    float rgbPercentScale;
    float chrominanceScale;

    /* The complete dimension pair is zero for an unbounded image. */
    __builtin_memcpy(&extentBits, &decodeControl->imageLimits, sizeof(extentBits));
    if (extentBits == 0) {
        codecWork = sw;
        codecWork->maximumY = codecWork->maximumX = JPEG_NO_LIMIT;
    } else {
        codecWork = sw;
        codecWork->maximumX = decodeControl->imageLimits.width - 1;
        codecWork->maximumY = decodeControl->imageLimits.height - 1;
    }
    codecWork->restartInterval = 0;
    codecWork->colorScaleMode = decodeControl->scaleMode;
    codecWork->blueScale = codecWork->greenScale = codecWork->redScale =
        codecWork->secondChromaScale = codecWork->firstChromaScale =
        codecWork->luminanceScale = 1.0f;

    switch (decodeControl->scaleMode) {
    case JPEG_SCALE_YUV:
        yuvPercentScale = JPEG_PERCENT_SCALE;
        sw->luminanceScale = I2F(decodeControl->luminancePercent) * yuvPercentScale;
        chrominanceScale = I2F(decodeControl->chrominancePercent) * yuvPercentScale;
        sw->firstChromaScale = sw->secondChromaScale = chrominanceScale;
        break;
    case JPEG_SCALE_RGB:
        rgbPercentScale = JPEG_PERCENT_SCALE;
        sw->redScale = I2F(decodeControl->redPercent) * rgbPercentScale;
        sw->greenScale = I2F(decodeControl->greenPercent) * rgbPercentScale;
        sw->blueScale = I2F(decodeControl->bluePercent) * rgbPercentScale;
        break;
    default:
        break;
    }
    cursor = decodeControl->jpegBytes;
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
            cursor = StartOfScan(cursor, decodeControl);
            break;
        default:
            cursor += (cursor[0] << 8) + cursor[1];
            break;
        }
    }
    decodeControl->imageLimits.width = sw->imageWidth;
    decodeControl->imageLimits.height = sw->imageHeight;
    decodeControl->consumedSize = cursor - decodeControl->jpegBytes;
    return 0;
}

/* Canned tables and VU0 constants of the codec (scaffold .data, address order). */
u8 ZigZag[64] = {
    0, 1, 8, 16, 9, 2, 3, 10,
    17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63,
};

float dctCosineTable[8][8] = {
    { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 1.38704002f, 1.17587996f, 0.785695016f, 0.275898993f, -0.275898993f, -0.785695016f, -1.17587996f, -1.38704002f },
    { 1.30656004f, 0.541195989f, -0.541195989f, -1.30656004f, -1.30656004f, -0.541195989f, 0.541195989f, 1.30656004f },
    { 1.17587996f, -0.275898993f, -1.38704002f, -0.785695016f, 0.785695016f, 1.38704002f, 0.275898993f, -1.17587996f },
    { 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f },
    { 0.785695016f, -1.38704002f, 0.275898993f, 1.17587996f, -1.17587996f, -0.275900006f, 1.38704002f, -0.785694003f },
    { 0.541195989f, -1.30656004f, 1.30656004f, -0.541195989f, -0.541195989f, 1.30656004f, -1.30656004f, 0.541195989f },
    { 0.275898993f, -0.785695016f, 1.17587996f, -1.38704002f, 1.38704002f, -1.17587996f, 0.785694003f, -0.27589801f },
};

u8 LQTableOrg_1[64] = {
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68, 109, 103, 77,
    24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99,
};

u8 CQTableOrg_2[64] = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
};

u8 headerALL[624] = {
    255, 216, 255, 224, 0, 16, 74, 70, 73, 70, 0, 1, 1, 0, 0, 100,
    0, 125, 0, 0, 255, 219, 0, 67, 0, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 219, 0, 67, 1, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 192,
    0, 17, 8, 255, 255, 255, 255, 3, 1, 255, 0, 2, 17, 1, 3, 17,
    1, 255, 196, 0, 31, 0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 255, 196, 0, 181, 16, 0, 2, 1, 3, 3, 2, 4, 3, 5,
    5, 4, 4, 0, 0, 1, 125, 1, 2, 3, 0, 4, 17, 5, 18, 33,
    49, 65, 6, 19, 81, 97, 7, 34, 113, 20, 50, 129, 145, 161, 8, 35,
    66, 177, 193, 21, 82, 209, 240, 36, 51, 98, 114, 130, 9, 10, 22, 23,
    24, 25, 26, 37, 38, 39, 40, 41, 42, 52, 53, 54, 55, 56, 57, 58,
    67, 68, 69, 70, 71, 72, 73, 74, 83, 84, 85, 86, 87, 88, 89, 90,
    99, 100, 101, 102, 103, 104, 105, 106, 115, 116, 117, 118, 119, 120, 121, 122,
    131, 132, 133, 134, 135, 136, 137, 138, 146, 147, 148, 149, 150, 151, 152, 153,
    154, 162, 163, 164, 165, 166, 167, 168, 169, 170, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 194, 195, 196, 197, 198, 199, 200, 201, 202, 210, 211, 212, 213,
    214, 215, 216, 217, 218, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 241,
    242, 243, 244, 245, 246, 247, 248, 249, 250, 255, 196, 0, 31, 1, 0, 3,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1,
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 255, 196, 0, 181, 17, 0,
    2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 119, 0,
    1, 2, 3, 17, 4, 5, 33, 49, 6, 18, 65, 81, 7, 97, 113, 19,
    34, 50, 129, 8, 20, 66, 145, 161, 177, 193, 9, 35, 51, 82, 240, 21,
    98, 114, 209, 10, 22, 36, 52, 225, 37, 241, 23, 24, 25, 26, 38, 39,
    40, 41, 42, 53, 54, 55, 56, 57, 58, 67, 68, 69, 70, 71, 72, 73,
    74, 83, 84, 85, 86, 87, 88, 89, 90, 99, 100, 101, 102, 103, 104, 105,
    106, 115, 116, 117, 118, 119, 120, 121, 122, 130, 131, 132, 133, 134, 135, 136,
    137, 138, 146, 147, 148, 149, 150, 151, 152, 153, 154, 162, 163, 164, 165, 166,
    167, 168, 169, 170, 178, 179, 180, 181, 182, 183, 184, 185, 186, 194, 195, 196,
    197, 198, 199, 200, 201, 202, 210, 211, 212, 213, 214, 215, 216, 217, 218, 226,
    227, 228, 229, 230, 231, 232, 233, 234, 242, 243, 244, 245, 246, 247, 248, 249,
    250, 255, 218, 0, 12, 3, 1, 0, 2, 17, 3, 17, 0, 63, 0, 0,
};

float imm0_3[40] = {
    1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    1.30656004f, 0.541195989f, 1.0f, -1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.541195989f, -1.30656004f, 1.38704002f, 1.17587996f,
    0.785695016f, 0.275898993f, 1.17587996f, -0.275898993f, -1.38704002f, -0.785695016f,
    0.785695016f, -1.38704002f, 0.275898993f, 1.17587996f, 0.275898993f, -0.785695016f,
    1.17587996f, -1.38704002f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f, -1.0f,
};

u32 imm1_4[178] = {
    0x3F800000u, 0x3F800000u, 0x3F800000u, 0, 0, 0xBEB0331Eu,
    0x3FE2D0E5u, 0, 0x3FB374BCu, 0xBF36D1E1u, 0, 0,
    0x43000000u, 0x43000000u, 0x43000000u, 0, 0, 0,
    0, 0, 255, 255, 255, 255,
    18432, 0x60000u, 14336, 0, 15360, 0,
    8128, 0, 24576, 0x3A000u, 24576, 0x3A000u,
    24576, 0x3A000u, 1024, 12288, 1152, 18432,
    1152, 18432, 1152, 18432, 1152, 18432,
    1152, 18432, 1152, 18432, 4096, 45056,
    8192, 0, 8192, 0, 20480, 0x40000u,
    64, 0, 64, 0, 36224, 0x62000u,
    2048, 0, 0xFFFFFFFFu, 0, 18432, 0x60000u,
    14336, 0, 15360, 0, 8128, 0,
    24576, 0x3A000u, 24576, 0x3A000u, 24576, 0x3A000u,
    4096, 0x20000u, 16384, 0x20000u, 16384, 0x20000u,
    16384, 0x20000u, 32768, 0x4B000u, 0xFFFFFFFFu, 0,
    18432, 0x60000u, 14336, 0, 15360, 0,
    8128, 0, 24576, 0x3A000u, 24576, 0x3A000u,
    24576, 0x3A000u, 4096, 0x20000u, 24576, 0x30000u,
    24576, 0x30000u, 0, 0, 32768, 0x4B000u,
    0xFFFFFFFFu, 0, 18432, 0x60000u, 14336, 0,
    15360, 0, 8128, 0, 24576, 0x3A000u,
    24576, 0x3A000u, 24576, 0x3A000u, 4096, 0x20000u,
    49152, 0x60000u, 0, 0, 0, 0,
    32768, 0x4B000u, 0xFFFFFFFFu, 0, 18432, 0x60000u,
    14336, 0, 15360, 0, 8128, 0,
    11264, 0x79000u, 6144, 0, 12288, 0,
    2048, 0, 4096, 0, 0, 0,
    8192, 0x30000u, 8192, 0x30000u, 8192, 0x30000u,
    8192, 0x30000u, 0xFFFFFFFFu, 0,
};

/* Two words of the .sdata area that follows sw; read by other TUs through sw. */
int D_005511A8 = 0x0064280F;
int D_005511AC = 0;
