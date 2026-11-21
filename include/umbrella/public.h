#ifndef INCLUDE_UMBRELLA_PUBLIC_H
#define INCLUDE_UMBRELLA_PUBLIC_H

void assertFailed(const char *message, const char *file, int line);

void sefResetDrawState(void);

void advanceState(void);

void xcBlockClear(void *clearedBlock);

int battleUnitCount(void);

void publishEntryRecords(void);

#endif /* INCLUDE_UMBRELLA_PUBLIC_H */
