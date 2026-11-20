/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). Shared types of src/main/sef.c; not included yet. */
#ifndef INCLUDE_MAIN_SEF_H
#define INCLUDE_MAIN_SEF_H

/* The image mapper entry read through svGetImageListItem (owner: sv_get_image_item_142.c). */
typedef struct SvTypeListEntry {
    void *data;                                 /* +0x00 */
    unsigned char unmodeled_04[0x0C - 0x04];
    unsigned short resourceKind;                /* +0x0C */
    unsigned char unmodeled_0E[0x12 - 0x0E];
    unsigned short basePointer;                 /* +0x12 */
    unsigned short bitsPerPixel;                /* +0x14 */
    unsigned short bufferWidthPixels;           /* +0x16 */
    unsigned short width;                       /* +0x18 */
    unsigned short height;                      /* +0x1A */
    unsigned short textureWidthExponent;        /* +0x1C */
    unsigned short textureHeightExponent;       /* +0x1E */
} SvTypeListEntry;

/* The current effect record (sefCurrentRecord): only the entry id at +0x08 is read here. */
typedef struct SefEffectRecord {
    unsigned char unmodeled_00[0x08];
    short entryId;                              /* +0x08 */
} SefEffectRecord;

#endif /* INCLUDE_MAIN_SEF_H */
