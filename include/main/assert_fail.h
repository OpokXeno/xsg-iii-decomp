#ifndef INCLUDE_MAIN_ASSERT_FAIL_H
#define INCLUDE_MAIN_ASSERT_FAIL_H

/* Assertion failure report: the original body only tail-calls assertExit. */
void assertFailed(const char *message, const char *file, int line);

#endif /* INCLUDE_MAIN_ASSERT_FAIL_H */
