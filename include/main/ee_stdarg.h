#ifndef INCLUDE_MAIN_EE_STDARG_H
#define INCLUDE_MAIN_EE_STDARG_H

/* The EE varargs of this build: every variadic argument takes one 8-byte slot of the register save area. */
typedef char *va_list;
#define va_start(ap, last) ((ap) = (va_list)__builtin_next_arg(last) - (8 - __builtin_args_info(2)) * 8)
#define va_end(ap) ((void)0)

#endif /* INCLUDE_MAIN_EE_STDARG_H */
