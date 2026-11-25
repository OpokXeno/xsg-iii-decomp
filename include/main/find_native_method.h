#ifndef INCLUDE_MAIN_FIND_NATIVE_METHOD_H
#define INCLUDE_MAIN_FIND_NATIVE_METHOD_H

typedef int s32;

/* A native method row: the script name, the native and an argument count; a table ends with a null name. */
typedef struct SbNativeMethod {
    const char *name;
    int (*func)();
    s32 argCount;
} SbNativeMethod;

#endif /* INCLUDE_MAIN_FIND_NATIVE_METHOD_H */
