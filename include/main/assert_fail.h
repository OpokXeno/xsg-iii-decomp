#ifndef INCLUDE_MAIN_ASSERT_FAIL_H
#define INCLUDE_MAIN_ASSERT_FAIL_H

/* Assertion failure report: the original body only tail-calls assertExit. */
void assertFailed(const char *message, const char *file, int line);
/* Binds the IOP sub module's RPC server, retrying until it answers. */
int func_00193600(void);

#endif /* INCLUDE_MAIN_ASSERT_FAIL_H */
