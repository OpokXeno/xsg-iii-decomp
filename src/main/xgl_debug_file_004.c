#include "common.h"
#include "shared.h"

#include "debug_break.h"

int xglDebFileSize(const char *name);
int xglDebFileRead(const char *name, void *fileBytes);
int xglDebFileWrite(const char *name, const void *fileBytes, int fileByteCount);
int printf(const char *format, ...);

/*
 * EE Timer 0.  T0_COUNT is the free-running counter and T0_MODE its control
 * word; bit 7 (0x80) of T0_MODE is CUE, which starts the count.
 */
#define T0_COUNT ((volatile u32 *)0x10000000)

#define T0_MODE ((volatile u32 *)0x10000010)
#define T0_MODE_CUE 0x80
#define T0_COUNT_MASK 0xffff

void xglTimer0Reset(int timerModeBits) {
    *T0_COUNT = 0;
    *T0_MODE = timerModeBits + T0_MODE_CUE;
}

int xglTimer0Get(void)
{
    return *T0_COUNT & T0_COUNT_MASK;
}

/* The two-letter language codes "jp", "jp", "us", "en", "de", "fr", "it", "es", "zh" and "ko", each read as one
 * halfword (first letter in the low byte). */
typedef struct XglLanguageCodes {
    unsigned short code[10];
} XglLanguageCodes;

extern const XglLanguageCodes xglLanguageCodeTable;
/* The language setting byte; 0 selects entry 2. */
extern unsigned char xglLanguageSetting[];

#define XGL_LANGUAGE_DEFAULT 2

/* Returns the code of the selected language; a non-null buffer receives its two bytes. */
unsigned short xglGetLanguageCode(char *buffer)
{
    XglLanguageCodes codes = xglLanguageCodeTable;
    unsigned int language = xglLanguageSetting[0];

    if (language == 0) {
        language = XGL_LANGUAGE_DEFAULT;
    }
    /* From here on language holds the code of the selected language. */
    language = codes.code[language];
    if (buffer != 0) {
        buffer[0] = language;
        buffer[1] = language >> 8;
    }
    return language;
}

void xglDebFileEmpty(void) {

}

/* Debug stubs: each passes a fixed "Don't use ..." message to printf. */
int xglDebFileSize(const char *name)
{
    printf("Don't use xglDebFileSize()\n");
    DEBUG_BREAK();
    return -1;
}

int xglDebFileRead(const char *name, void *fileBytes)
{
    printf("Don't use xglDebFileRead()\n");
    DEBUG_BREAK();
    return -1;
}

int xglDebFileWrite(const char *name, const void *fileBytes, int fileByteCount)
{
    printf("Don't use xglDebFileWrite()\n");
    DEBUG_BREAK();
    return -1;
}
