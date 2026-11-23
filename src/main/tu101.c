#include "common.h"
#include "shared.h"

/* One entry of the name table at gNameMapTable: a name string and the string it maps to.
 * The table ends with an entry whose name is 0. */
typedef struct NameMapEntry {
    char *name;       /* +0x00 */
    char *mappedName; /* +0x04 */
} NameMapEntry;

extern NameMapEntry gNameMapTable[];
extern u64 gGameLoopRuntimeStorage;
char *func_001B6DA0(int requestId);
int func_0030E68C(char *a, char *b);

/* Maps a name through the table; with the runtime bits set, the first 264 entries
 * answer with the fixed name of request 0x0300001B instead. */
char *nameMapLookup(char *name) {
    NameMapEntry *nameMapRow;
    char *fixedName;
    int nameRow;

    nameMapRow = gNameMapTable;
    fixedName = func_001B6DA0(0x0300001B);
    nameRow = 0;
    for (; nameMapRow->name != 0; nameMapRow++, nameRow++) {
        if (func_0030E68C(nameMapRow->name, name) == 0) {
            if ((gGameLoopRuntimeStorage & 0x0800000002000000ULL) != 0 && nameRow < 264) {
                return fixedName;
            }
            return nameMapRow->mappedName;
        }
    }
    return 0;
}

char *func_001B6CA8(char *name, int mode);

/* Request name -> mapped name -> func_001B6CA8 with mode 0x03000000. */
char *requestNameMappedPathGet(int requestId) {
    char *requestName;
    char *mappedName;

    requestName = func_001B6DA0(requestId);
    if (requestName == 0) {
        return 0;
    }
    mappedName = nameMapLookup(requestName);
    if (mappedName == 0) {
        return 0;
    }
    do {
        return func_001B6CA8(mappedName, 0x03000000);
    } while (0);
}

/* Same as requestNameMappedPathGet with mode 0x03010000. */
char *func_001D0258(int requestId) {
    char *requestName;
    char *mappedName;

    requestName = func_001B6DA0(requestId);
    if (requestName == 0) {
        return 0;
    }
    mappedName = nameMapLookup(requestName);
    if (mappedName == 0) {
        return 0;
    }
    do {
        return func_001B6CA8(mappedName, 0x03010000);
    } while (0);
}
