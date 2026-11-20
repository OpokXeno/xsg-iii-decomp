#ifndef INCLUDE_MAIN_CFEPLUGIN_H
#define INCLUDE_MAIN_CFEPLUGIN_H

/* Declarations owned by main/cfePlugIn and shared with other TUs. */

#include "shared.h"

/* Both set flag bits of the word at +0x698 of D_008C28B0: func_0019EBB0 ORs 0x10 when the argument is nonzero, func_0019EBD0 ORs 0x80000. */
void func_0019EBB0(int enable);
void func_0019EBD0(void);

#endif
