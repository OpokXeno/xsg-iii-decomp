#include "common.h"
#include "shared.h"
/* The key type is the low 12 bits of typeFlags. */
#define XAC_KEY_TYPE_MASK 0xfff
#include "main/TiMRPackage.h"
#include "main/TiXacAnimCurve.h"

/* Transform channel header: its first halfword holds the nine channel enable bits (0x1 to 0x100) and transformModeFlags the mode bit (0x1), the weight is at +4. */
typedef struct TiXacTransformHeader {
    u16 channelFlags;
    u16 transformModeFlags;
    f32 weight;
} TiXacTransformHeader;

extern const char xacAssertFile[];
/* Evaluates a keyframe curve at curveParam: reads keys from *keyCursor, advances it, interpolates through xacEvalLinearKeys; asserts on an illegal curve type. */
f32 xacEvalCurve(f32 curveParam, u8 **keyCursor);
void xacEvalTransform(f32 *channelValues, u8 **keyCursor, f32 curveParam);
f32 *xacFindCurveKey(int *hasKeyValue, XacCurveKeyTable *curveKeyTable, f32 curveParam);
void assertFailed(const char *message, const char *file, int line);
f32 xacEvalCubicKeys(f32 time, TiXacKey *key, TiXacKey *nextKey);
f32 xacEvalLinearKeys(f32 time, TiXacKey *key, TiXacKey *nextKey);

/* Channel bits of a transform header (channelFlags), in the order of the nine values: translation X, Y, Z (0 to 2),
 * rotation X, Y, Z (3 to 5), scale X, Y, Z (6 to 8); the same order as TiHrcDynamics. */
enum XacChannelBit {
    XAC_CHANNEL_TX = 0x1,
    XAC_CHANNEL_TY = 0x2,
    XAC_CHANNEL_TZ = 0x4,
    XAC_CHANNEL_RX = 0x8,
    XAC_CHANNEL_RY = 0x10,
    XAC_CHANNEL_RZ = 0x20,
    XAC_CHANNEL_SX = 0x40,
    XAC_CHANNEL_SY = 0x80,
    XAC_CHANNEL_SZ = 0x100
};

/* The transform header at a cursor (*keyCursor): the single typed view of the cursor bytes. */
#define XAC_HEADER_AT(cursor) ((TiXacTransformHeader *) *(cursor))

/* Key type word: XAC_KEY_END marks the last key of a curve, XAC_KEY_LINEAR a linear segment (the type itself is XAC_KEY_TYPE_MASK). */
#define XAC_KEY_END 0x4000
#define XAC_KEY_LINEAR 0x8000

extern const char xacAssertFile[24];
extern const char xacHermiteKeyAssertText[24];
extern const char xacHermiteKeyAssertExpr[24];
extern const char xacIllegalCurveTypeAssertText[24];
extern const char xacIllegalCurveTypeAssertExpr[32];
extern f32 gXacOutTangentScale;
extern const char xacUnreachableAssertText[];
extern const char D_00542C30[];

/* Reads one curve key's out-tangent value by key type: 0 gives 0.0, 2 a float at +4, 3 a u16 at +6 times 1e-4, 5 a float at +8,
 * 6 a signed 4.12 fixed-point halfword at +4; other types assert. */
f32 xacGetKeyOutTangentAngle(TiXacKey *key) {
    s16 fixed;
    int biased;
    int whole;
    s16 fraction;

    switch (key->typeFlags & XAC_KEY_TYPE_MASK) {
    case 0:
        return 0.0f;
    case 3:
        return key->payload.halfwords[1] * gXacOutTangentScale;
    case 5:
        return key->payload.value[1];
    case 2:
        return key->payload.value[0];
    case 6:
        fixed = key->payload.halfwords[0];
        biased = fixed < 0 ? fixed + 0xFFF : fixed;
        whole = biased >> 12;
        fraction = fixed - (whole << 12);
        return whole + fraction * (1.0f / 4096.0f);
    default:
        assertFailed(xacUnreachableAssertText, D_00542C30, 37);
        return 0.0f;
    }
}

extern f32 gXacInTangentScale;

/* Reads one curve key's in-tangent value by key type: 0 gives 0.0, 2 and 5 a float at +4, 3 a u16 at +4 times 1e-4,
 * 6 a signed 4.12 fixed-point halfword at +4; other types assert. */
f32 xacGetKeyInTangentAngle(TiXacKey *key) {
    s16 fixed;
    int biased;
    int whole;
    s16 fraction;

    switch (key->typeFlags & XAC_KEY_TYPE_MASK) {
    case 0:
        return 0.0f;
    case 3:
        return key->payload.halfwords[0] * gXacInTangentScale;
    case 5:
        return key->payload.value[0];
    case 2:
        return key->payload.value[0];
    case 6:
        fixed = key->payload.halfwords[0];
        biased = fixed < 0 ? fixed + 0xFFF : fixed;
        whole = biased >> 12;
        fraction = fixed - (whole << 12);
        return whole + fraction * (1.0f / 4096.0f);
    default:
        assertFailed(xacUnreachableAssertText, D_00542C30, 77);
        return 0.0f;
    }
}

extern const char xacKeyHeaderAssertText[];

/* Returns one float of a key header by its key type: types 0 and 1 read +4, 2 and 3 read +8, 5 reads +C, 6 a signed 4.12 halfword
 * at +6, other types give 0.0; asserts on a null pointer. */
f32 xacGetKeyValue(TiXacKey *pKeyHeader) {
    s16 fixed;
    int biased;
    int whole;
    s16 fraction;

    if (pKeyHeader == 0) {
        assertFailed(xacKeyHeaderAssertText, D_00542C30, 88);
    }
    switch (pKeyHeader->typeFlags & XAC_KEY_TYPE_MASK) {
    case 5:
        return pKeyHeader->payload.value[2];
    case 2:
    case 3:
        return pKeyHeader->payload.value[1];
    case 0:
    case 1:
        return pKeyHeader->payload.value[0];
    case 6:
        fixed = pKeyHeader->payload.halfwords[1];
        biased = fixed < 0 ? fixed + 0xFFF : fixed;
        whole = biased >> 12;
        fraction = fixed - (whole << 12);
        return whole + fraction * (1.0f / 4096.0f);
    default:
        return 0.0f;
    }
}

extern const char D_00542CC0[];
extern const char D_00542CD8[];
extern const char D_00542CF0[];
extern const char D_00542D08[];
f32 xacEvalCubicKeys(f32 time, TiXacKey *key, TiXacKey *nextKey);

/* Evaluates a keyframe curve at curveParam: reads keys from *keyCursor, advances it, interpolates through xacEvalLinearKeys; asserts on an illegal curve type. */
f32 xacEvalCurve(f32 curveParam, u8 **keyCursor) {
    TiXacKey *key;
    TiXacKey *lastKey;
    f32 *keyPayloadFloats;
    f32 curveValue;
    f32 evalTime;
    int searching;

    key = (TiXacKey *) *keyCursor;
    curveValue = 0.0f;
    evalTime = curveParam;
    searching = 1;
    if (evalTime < key->origin) {
        evalTime = key->origin;
    }
    do {
        switch (key->typeFlags & XAC_KEY_TYPE_MASK) {
        case 1: {
            TiXacKey *nextKey;
            keyPayloadFloats = key->payload.value;
            nextKey = (TiXacKey *) &key->payload.halfwords[2];
            if (searching != 0) {
                if (key->typeFlags & XAC_KEY_END) {
                    curveValue = keyPayloadFloats[0];
                    searching = 0;
                } else if (key->origin <= evalTime) {
                    if (evalTime < nextKey->origin) {
                        curveValue = xacEvalLinearKeys(evalTime, key, nextKey);
                        searching = 0;
                    }
                }
            }
            lastKey = key;
            key = nextKey;
            break;
        }
        case 4:
            if (D_00542CC0 != 0) {
                assertFailed(D_00542CD8, D_00542C30, 169);
            }
            break;
        case 2: {
            TiXacKey *nextKey;
            keyPayloadFloats = key->payload.value;
            nextKey = (TiXacKey *) &key->payload.halfwords[4];
            if (searching != 0) {
                if (key->typeFlags & XAC_KEY_END) {
                    curveValue = keyPayloadFloats[1];
                    searching = 0;
                } else if (key->origin <= evalTime) {
                    if (evalTime < nextKey->origin) {
                        searching = 0;
                        if ((key->typeFlags & XAC_KEY_LINEAR) && nextKey->typeFlags == 1) {
                            curveValue = xacEvalLinearKeys(evalTime, key, nextKey);
                        } else {
                            curveValue = xacEvalCubicKeys(evalTime, key, nextKey);
                        }
                    }
                }
            }
            lastKey = key;
            key = nextKey;
            break;
        }
        case 6: {
            TiXacKey *nextKey;
            u16 *halfwords;
            halfwords = key->payload.halfwords;
            nextKey = (TiXacKey *) &key->payload.halfwords[2];
            if (searching != 0) {
                if (key->typeFlags & XAC_KEY_END) {
                    curveValue = (s16) halfwords[1];
                    searching = 0;
                } else if (key->origin <= evalTime) {
                    if (evalTime < nextKey->origin) {
                        searching = 0;
                        if ((key->typeFlags & XAC_KEY_LINEAR) && nextKey->typeFlags == 1) {
                            curveValue = xacEvalLinearKeys(evalTime, key, nextKey);
                        } else {
                            curveValue = xacEvalCubicKeys(evalTime, key, nextKey);
                        }
                    }
                }
            }
            lastKey = key;
            key = nextKey;
            break;
        }
        case 3: {
            TiXacKey *nextKey;
            keyPayloadFloats = key->payload.value;
            nextKey = (TiXacKey *) &key->payload.halfwords[4];
            if (searching != 0) {
                if (key->typeFlags & XAC_KEY_END) {
                    curveValue = keyPayloadFloats[1];
                    searching = 0;
                } else if (key->origin <= evalTime) {
                    if (evalTime < nextKey->origin) {
                        searching = 0;
                        if ((key->typeFlags & XAC_KEY_LINEAR) && nextKey->typeFlags == 1) {
                            curveValue = xacEvalLinearKeys(evalTime, key, nextKey);
                        } else {
                            curveValue = xacEvalCubicKeys(evalTime, key, nextKey);
                        }
                    }
                }
            }
            lastKey = key;
            key = nextKey;
            break;
        }
        case 5: {
            TiXacKey *nextKey;
            keyPayloadFloats = key->payload.value;
            nextKey = key + 1;
            if (searching != 0) {
                if (key->typeFlags & XAC_KEY_END) {
                    curveValue = keyPayloadFloats[2];
                    searching = 0;
                } else if (key->origin <= evalTime) {
                    if (evalTime < nextKey->origin) {
                        searching = 0;
                        if ((key->typeFlags & XAC_KEY_LINEAR) && nextKey->typeFlags == 1) {
                            curveValue = xacEvalLinearKeys(evalTime, key, nextKey);
                        } else {
                            curveValue = xacEvalCubicKeys(evalTime, key, nextKey);
                        }
                    }
                }
            }
            lastKey = key;
            key = nextKey;
            break;
        }
        case 0: {
            TiXacKey *nextKey;
            keyPayloadFloats = key->payload.value;
            nextKey = (TiXacKey *) &key->payload.halfwords[2];
            if (searching != 0) {
                if (key->typeFlags & XAC_KEY_END) {
                    curveValue = keyPayloadFloats[0];
                    searching = 0;
                } else if (key->origin <= evalTime) {
                    if (evalTime < nextKey->origin) {
                        searching = 0;
                        if ((key->typeFlags & XAC_KEY_LINEAR) && nextKey->typeFlags == 1) {
                            curveValue = xacEvalLinearKeys(evalTime, key, nextKey);
                        } else {
                            curveValue = xacEvalCubicKeys(evalTime, key, nextKey);
                        }
                    }
                }
            }
            lastKey = key;
            key = nextKey;
            break;
        }
        default:
            if (D_00542CF0 != 0) {
                assertFailed(D_00542D08, D_00542C30, 335);
            }
            lastKey = 0;
            break;
        }
    } while (!(lastKey->typeFlags & XAC_KEY_END));
    *keyCursor = (u8 *) key;
    return curveValue;
}

/* Evaluates the nine channels of a transform header at *keyCursor into channelValues[0..8]: a set channel bit reads a curve,
 * a clear bit takes the joint default; the header's mode bit sets or clears the joint's object mode bit. A null cursor
 * takes every default. */
void xacEvalTransJoint(f32 *channelValues, TiHrcJoint *pJoint, u8 **keyCursor, f32 curveParam) {
    TiXacTransformHeader *transformHeader;
    u8 *cursor;

    if (*keyCursor != 0) {
        transformHeader = XAC_HEADER_AT(keyCursor);
        if (transformHeader->transformModeFlags & 1) {
            pJoint->flags |= 2;
        } else {
            pJoint->flags &= ~2;
        }
        cursor = (u8 *) &transformHeader->weight;
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TX) {
            channelValues[0] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[0] = pJoint->translationDefault[0];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TY) {
            channelValues[1] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[1] = pJoint->translationDefault[1];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TZ) {
            channelValues[2] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[2] = pJoint->translationDefault[2];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RX) {
            channelValues[3] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[3] = pJoint->rotationDefault[0];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RY) {
            channelValues[4] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[4] = pJoint->rotationDefault[1];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RZ) {
            channelValues[5] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[5] = pJoint->rotationDefault[2];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SX) {
            channelValues[6] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[6] = pJoint->scaleDefault[0];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SY) {
            channelValues[7] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[7] = pJoint->scaleDefault[1];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SZ) {
            channelValues[8] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[8] = pJoint->scaleDefault[2];
        }
        *keyCursor = cursor;
    } else {
        channelValues[0] = pJoint->translationDefault[0];
        pJoint->flags &= ~2;
        channelValues[1] = pJoint->translationDefault[1];
        channelValues[2] = pJoint->translationDefault[2];
        channelValues[3] = pJoint->rotationDefault[0];
        channelValues[4] = pJoint->rotationDefault[1];
        channelValues[5] = pJoint->rotationDefault[2];
        channelValues[6] = pJoint->scaleDefault[0];
        channelValues[7] = pJoint->scaleDefault[1];
        channelValues[8] = pJoint->scaleDefault[2];
    }
}

/* Evaluates nine keyed channels into channelValues[0..8] from the transform header at *keyCursor (flag bits 0x1 to 0x100); channels 6 to 8 default to 1.0, the rest to 0.0; writes the advanced cursor back. */
void xacEvalTransform(f32 *channelValues, u8 **keyCursor, f32 curveParam) {
    u8 *cursor;

    if (*keyCursor != 0) {
        cursor = *keyCursor + 4;
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TX) {
            channelValues[0] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[0] = 0.0f;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TY) {
            channelValues[1] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[1] = 0.0f;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TZ) {
            channelValues[2] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[2] = 0.0f;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RX) {
            channelValues[3] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[3] = 0.0f;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RY) {
            channelValues[4] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[4] = 0.0f;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RZ) {
            channelValues[5] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[5] = 0.0f;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SX) {
            channelValues[6] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[6] = 1.0f;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SY) {
            channelValues[7] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[7] = 1.0f;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SZ) {
            channelValues[8] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[8] = 1.0f;
        }
        *keyCursor = cursor;
    }
}

/* Evaluates the nine channels of a locator's transform header into channelValues[0..8], as the joint evaluator does. */
void xacEvalTransLocator(f32 *channelValues, TiHrcLocator *pLocator, u8 **keyCursor, f32 curveParam) {
    TiXacTransformHeader *transformHeader;
    u8 *cursor;

    if (*keyCursor != 0) {
        transformHeader = XAC_HEADER_AT(keyCursor);
        if (transformHeader->transformModeFlags & 1) {
            pLocator->flags |= 2;
        } else {
            pLocator->flags &= ~2;
        }
        cursor = (u8 *) &transformHeader->weight;
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TX) {
            channelValues[0] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[0] = pLocator->translationDefault[0];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TY) {
            channelValues[1] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[1] = pLocator->translationDefault[1];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TZ) {
            channelValues[2] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[2] = pLocator->translationDefault[2];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RX) {
            channelValues[3] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[3] = pLocator->rotationDefault[0];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RY) {
            channelValues[4] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[4] = pLocator->rotationDefault[1];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RZ) {
            channelValues[5] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[5] = pLocator->rotationDefault[2];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SX) {
            channelValues[6] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[6] = pLocator->scaleDefault[0];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SY) {
            channelValues[7] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[7] = pLocator->scaleDefault[1];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SZ) {
            channelValues[8] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[8] = pLocator->scaleDefault[2];
        }
        *keyCursor = cursor;
    } else {
        channelValues[0] = pLocator->translationDefault[0];
        pLocator->flags &= ~2;
        channelValues[1] = pLocator->translationDefault[1];
        channelValues[2] = pLocator->translationDefault[2];
        channelValues[3] = pLocator->rotationDefault[0];
        channelValues[4] = pLocator->rotationDefault[1];
        channelValues[5] = pLocator->rotationDefault[2];
        channelValues[6] = pLocator->scaleDefault[0];
        channelValues[7] = pLocator->scaleDefault[1];
        channelValues[8] = pLocator->scaleDefault[2];
    }
}

/* Evaluates the nine channels of a skip node's transform header into channelValues[0..8], as the joint evaluator does. */
void xacEvalTransFace(f32 *channelValues, TiHrcSkip *pSkip, u8 **keyCursor, f32 curveParam) {
    TiXacTransformHeader *transformHeader;
    u8 *cursor;

    if (*keyCursor != 0) {
        transformHeader = XAC_HEADER_AT(keyCursor);
        if (transformHeader->transformModeFlags & 1) {
            pSkip->flags |= 2;
        } else {
            pSkip->flags &= ~2;
        }
        cursor = (u8 *) &transformHeader->weight;
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TX) {
            channelValues[0] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[0] = pSkip->translationDefault[0];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TY) {
            channelValues[1] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[1] = pSkip->translationDefault[1];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TZ) {
            channelValues[2] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[2] = pSkip->translationDefault[2];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RX) {
            channelValues[3] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[3] = pSkip->rotationDefault[0];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RY) {
            channelValues[4] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[4] = pSkip->rotationDefault[1];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RZ) {
            channelValues[5] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[5] = pSkip->rotationDefault[2];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SX) {
            channelValues[6] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[6] = pSkip->scaleDefault[0];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SY) {
            channelValues[7] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[7] = pSkip->scaleDefault[1];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SZ) {
            channelValues[8] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[8] = pSkip->scaleDefault[2];
        }
        *keyCursor = cursor;
    } else {
        channelValues[0] = pSkip->translationDefault[0];
        pSkip->flags &= ~2;
        channelValues[1] = pSkip->translationDefault[1];
        channelValues[2] = pSkip->translationDefault[2];
        channelValues[3] = pSkip->rotationDefault[0];
        channelValues[4] = pSkip->rotationDefault[1];
        channelValues[5] = pSkip->rotationDefault[2];
        channelValues[6] = pSkip->scaleDefault[0];
        channelValues[7] = pSkip->scaleDefault[1];
        channelValues[8] = pSkip->scaleDefault[2];
    }
}

/* Evaluates the nine channels of a dynamics node from the transform header at *keyCursor into channelValues[0..8]; the header's
 * weight is stored in the node. The curve cursor starts past the weight. A null cursor takes every default and clears
 * the weight. */
void xacEvalTransDynamics(f32 *channelValues, TiHrcDynamics *pDyn, u8 **keyCursor, f32 curveParam) {
    TiXacTransformHeader *transformHeader;
    u8 *cursor;
    f32 *weightSrc;

    if (*keyCursor != 0) {
        transformHeader = XAC_HEADER_AT(keyCursor);
        cursor = (u8 *) &transformHeader[1];
        weightSrc = &transformHeader->weight;
        if (transformHeader->transformModeFlags & 1) {
            pDyn->flags |= 2;
        } else {
            pDyn->flags &= ~2;
        }
        pDyn->weight = *weightSrc;
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TX) {
            channelValues[0] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[0] = pDyn->translationDefault.x;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TY) {
            channelValues[1] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[1] = pDyn->translationDefault.y;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TZ) {
            channelValues[2] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[2] = pDyn->translationDefault.z;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RX) {
            channelValues[3] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[3] = pDyn->rotationDefault[0];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RY) {
            channelValues[4] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[4] = pDyn->rotationDefault[1];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RZ) {
            channelValues[5] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[5] = pDyn->rotationDefault[2];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SX) {
            channelValues[6] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[6] = pDyn->scaleDefault[0];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SY) {
            channelValues[7] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[7] = pDyn->scaleDefault[1];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SZ) {
            channelValues[8] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[8] = pDyn->scaleDefault[2];
        }
        *keyCursor = cursor;
    } else {
        pDyn->weight = 0.0f;
        channelValues[0] = pDyn->translationDefault.x;
        channelValues[1] = pDyn->translationDefault.y;
        pDyn->flags &= ~2;
        channelValues[2] = pDyn->translationDefault.z;
        channelValues[3] = pDyn->rotationDefault[0];
        channelValues[4] = pDyn->rotationDefault[1];
        channelValues[5] = pDyn->rotationDefault[2];
        channelValues[6] = pDyn->scaleDefault[0];
        channelValues[7] = pDyn->scaleDefault[1];
        channelValues[8] = pDyn->scaleDefault[2];
    }
}

void xacEvalTransDynamicsUnweighted(f32 *channelValues, TiHrcDynamics *pDyn, u8 **keyCursor, f32 curveParam) {
    TiXacTransformHeader *transformHeader;
    u8 *cursor;

    pDyn->weight = 0.0f;
    if (*keyCursor != 0) {
        transformHeader = XAC_HEADER_AT(keyCursor);
        if (transformHeader->transformModeFlags & 1) {
            pDyn->flags |= 2;
        } else {
            pDyn->flags &= ~2;
        }
        cursor = (u8 *) &transformHeader->weight;
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TX) {
            channelValues[0] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[0] = pDyn->translationDefault.x;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TY) {
            channelValues[1] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[1] = pDyn->translationDefault.y;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_TZ) {
            channelValues[2] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[2] = pDyn->translationDefault.z;
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RX) {
            channelValues[3] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[3] = pDyn->rotationDefault[0];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RY) {
            channelValues[4] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[4] = pDyn->rotationDefault[1];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_RZ) {
            channelValues[5] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[5] = pDyn->rotationDefault[2];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SX) {
            channelValues[6] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[6] = pDyn->scaleDefault[0];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SY) {
            channelValues[7] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[7] = pDyn->scaleDefault[1];
        }
        if (XAC_HEADER_AT(keyCursor)->channelFlags & XAC_CHANNEL_SZ) {
            channelValues[8] = xacEvalCurve(curveParam, &cursor);
        } else {
            channelValues[8] = pDyn->scaleDefault[2];
        }
        *keyCursor = cursor;
    } else {
        channelValues[0] = pDyn->translationDefault.x;
        pDyn->flags &= ~2;
        channelValues[1] = pDyn->translationDefault.y;
        channelValues[2] = pDyn->translationDefault.z;
        channelValues[3] = pDyn->rotationDefault[0];
        channelValues[4] = pDyn->rotationDefault[1];
        channelValues[5] = pDyn->rotationDefault[2];
        channelValues[6] = pDyn->scaleDefault[0];
        channelValues[7] = pDyn->scaleDefault[1];
        channelValues[8] = pDyn->scaleDefault[2];
    }
}

/* Evaluates six pose channels at curveParam into channelValues[0..5] from the cursor at *ppJointHeader; returns the advanced cursor. */
u8 *xacEvalJointPose(f32 curveParam, f32 *channelValues, u8 **ppJointHeader) {
    u8 *cursor;

    cursor = *ppJointHeader + 4;
    channelValues[0] = xacEvalCurve(curveParam, &cursor);
    channelValues[1] = xacEvalCurve(curveParam, &cursor);
    channelValues[2] = xacEvalCurve(curveParam, &cursor);
    channelValues[3] = xacEvalCurve(curveParam, &cursor);
    channelValues[4] = xacEvalCurve(curveParam, &cursor);
    channelValues[5] = xacEvalCurve(curveParam, &cursor);
    *ppJointHeader = cursor;
    return cursor;
}

/* Evaluates one joint channel at curveParam into *channelValues, reading the cursor from *ppJointHeader past its 4-byte header; returns the advanced cursor. */
u8 *xacEvalJointValue(f32 curveParam, f32 *channelValues, u8 **ppJointHeader) {
    u8 *cursor;

    cursor = *ppJointHeader + 4;
    *channelValues = xacEvalCurve(curveParam, &cursor);
    *ppJointHeader = cursor;
    return cursor;
}

u8 *xacReadJointCurves24(f32 *channelValues, u8 **ppJointHeader, f32 curveParam) {
    u8 *cursor;

    cursor = *ppJointHeader + 4;
    channelValues[0] = xacEvalCurve(curveParam, &cursor);
    channelValues[1] = xacEvalCurve(curveParam, &cursor);
    channelValues[2] = xacEvalCurve(curveParam, &cursor);
    channelValues[3] = xacEvalCurve(curveParam, &cursor);
    channelValues[4] = xacEvalCurve(curveParam, &cursor);
    channelValues[5] = xacEvalCurve(curveParam, &cursor);
    channelValues[6] = xacEvalCurve(curveParam, &cursor);
    channelValues[7] = xacEvalCurve(curveParam, &cursor);
    channelValues[8] = xacEvalCurve(curveParam, &cursor);
    channelValues[9] = xacEvalCurve(curveParam, &cursor);
    channelValues[10] = xacEvalCurve(curveParam, &cursor);
    channelValues[11] = xacEvalCurve(curveParam, &cursor);
    channelValues[12] = xacEvalCurve(curveParam, &cursor);
    channelValues[13] = xacEvalCurve(curveParam, &cursor);
    channelValues[14] = xacEvalCurve(curveParam, &cursor);
    channelValues[15] = xacEvalCurve(curveParam, &cursor);
    channelValues[16] = xacEvalCurve(curveParam, &cursor);
    channelValues[17] = xacEvalCurve(curveParam, &cursor);
    channelValues[18] = xacEvalCurve(curveParam, &cursor);
    channelValues[19] = xacEvalCurve(curveParam, &cursor);
    channelValues[20] = xacEvalCurve(curveParam, &cursor);
    channelValues[21] = xacEvalCurve(curveParam, &cursor);
    channelValues[22] = xacEvalCurve(curveParam, &cursor);
    channelValues[23] = xacEvalCurve(curveParam, &cursor);
    *ppJointHeader = cursor;
    return cursor;
}

/* Evaluates nine curve channels at curveParam into channelValues[0..8] from the cursor just past the joint header at jointHeader. */
void xacEvalJointTransformCurves(f32 curveParam, f32 *channelValues, u8 *jointHeader) {
    u8 *cursor;

    cursor = jointHeader + 4;
    channelValues[0] = xacEvalCurve(curveParam, &cursor);
    channelValues[1] = xacEvalCurve(curveParam, &cursor);
    channelValues[2] = xacEvalCurve(curveParam, &cursor);
    channelValues[3] = xacEvalCurve(curveParam, &cursor);
    channelValues[4] = xacEvalCurve(curveParam, &cursor);
    channelValues[5] = xacEvalCurve(curveParam, &cursor);
    channelValues[6] = xacEvalCurve(curveParam, &cursor);
    channelValues[7] = xacEvalCurve(curveParam, &cursor);
    channelValues[8] = xacEvalCurve(curveParam, &cursor);
}

void xacReadWindDirCurves(f32 *channelValues, u8 **keyCursor, f32 curveParam) {
    u8 *cursor;

    cursor = *keyCursor + 4;
    channelValues[0] = xacEvalCurve(curveParam, &cursor);
    if (channelValues[0] == 0.0f) {
        return;
    }
    channelValues[1] = xacEvalCurve(curveParam, &cursor);
    channelValues[2] = xacEvalCurve(curveParam, &cursor);
    channelValues[3] = xacEvalCurve(curveParam, &cursor);
    channelValues[4] = xacEvalCurve(curveParam, &cursor);
    channelValues[5] = xacEvalCurve(curveParam, &cursor);
    channelValues[6] = xacEvalCurve(curveParam, &cursor);
    channelValues[7] = xacEvalCurve(curveParam, &cursor);
    channelValues[8] = xacEvalCurve(curveParam, &cursor);
    channelValues[9] = xacEvalCurve(curveParam, &cursor);
    channelValues[10] = xacEvalCurve(curveParam, &cursor);
    channelValues[11] = xacEvalCurve(curveParam, &cursor);
    *keyCursor = cursor;
}

void xacReadWindPointCurves(f32 *channelValues, u8 **keyCursor, f32 curveParam) {
    u8 *cursor;

    cursor = *keyCursor + 4;
    channelValues[0] = xacEvalCurve(curveParam, &cursor);
    if (channelValues[0] == 0.0f) {
        return;
    }
    channelValues[1] = xacEvalCurve(curveParam, &cursor);
    channelValues[2] = xacEvalCurve(curveParam, &cursor);
    channelValues[3] = xacEvalCurve(curveParam, &cursor);
    channelValues[4] = xacEvalCurve(curveParam, &cursor);
    channelValues[5] = xacEvalCurve(curveParam, &cursor);
    channelValues[6] = xacEvalCurve(curveParam, &cursor);
    channelValues[7] = xacEvalCurve(curveParam, &cursor);
    channelValues[8] = xacEvalCurve(curveParam, &cursor);
    channelValues[9] = xacEvalCurve(curveParam, &cursor);
    channelValues[10] = xacEvalCurve(curveParam, &cursor);
    channelValues[11] = xacEvalCurve(curveParam, &cursor);
    *keyCursor = cursor;
}

INCLUDE_ASM("asm/main/nonmatchings/TiXacAnimCurve", xacReadJointCurves12Padded);

/* Reads fifteen curve channelValues from a joint header, starting 4 bytes in, and advances *ppJointHeader; returns the advanced cursor. */
u8 *xacReadJointCurves15(f32 *channelValues, u8 **ppJointHeader, f32 curveParam) {
    u8 *cursor;

    cursor = *ppJointHeader + 4;
    channelValues[0] = xacEvalCurve(curveParam, &cursor);
    channelValues[1] = xacEvalCurve(curveParam, &cursor);
    channelValues[2] = xacEvalCurve(curveParam, &cursor);
    channelValues[3] = xacEvalCurve(curveParam, &cursor);
    channelValues[4] = xacEvalCurve(curveParam, &cursor);
    channelValues[5] = xacEvalCurve(curveParam, &cursor);
    channelValues[6] = xacEvalCurve(curveParam, &cursor);
    channelValues[7] = xacEvalCurve(curveParam, &cursor);
    channelValues[8] = xacEvalCurve(curveParam, &cursor);
    channelValues[9] = xacEvalCurve(curveParam, &cursor);
    channelValues[10] = xacEvalCurve(curveParam, &cursor);
    channelValues[11] = xacEvalCurve(curveParam, &cursor);
    channelValues[12] = xacEvalCurve(curveParam, &cursor);
    channelValues[13] = xacEvalCurve(curveParam, &cursor);
    channelValues[14] = xacEvalCurve(curveParam, &cursor);
    *ppJointHeader = cursor;
    return cursor;
}

u8 *xacReadJointValueAndStart(f32 *channelValues, int unused_arg1, u8 **ppJointHeader, f32 curveParam) {
    u8 *cursor;

    cursor = *ppJointHeader + 4;
    channelValues[0] = xacEvalCurve(curveParam, &cursor);
    channelValues[1] = xacEvalCurve(curveParam, &cursor);
    *ppJointHeader = cursor;
    return cursor;
}

/* Scans a key table (count word, then key times) for the key segment around curveParam; writes a 0/1 flag to its first argument and returns the key pointer. */
f32 *xacFindCurveKey(int *hasKeyValue, XacCurveKeyTable *curveKeyTable, f32 curveParam) {
    f32 *pKey;
    f32 *keyValue;
    f32 prevTime;
    int keyIndex;

    prevTime = 0.0f;
    if (curveKeyTable->count <= 0) {
        return curveKeyTable->keys;
    }
    pKey = curveKeyTable->keys;
    for (keyIndex = 0; keyIndex < curveKeyTable->count; keyIndex++) {
        keyValue = pKey + 1;
        if ((prevTime < curveParam && curveParam <= *pKey) || keyIndex == 0) {
            if (*keyValue != 0.0f) {
                *hasKeyValue = 1;
            } else {
                *hasKeyValue = 0;
            }
        }
        prevTime = *pKey;
        pKey = keyValue + 1;
    }
    if (prevTime < curveParam) {
        if (*keyValue != 0.0f) {
            *hasKeyValue = 1;
        } else {
            *hasKeyValue = 0;
        }
    }
    return pKey;
}
