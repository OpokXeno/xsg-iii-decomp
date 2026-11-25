#ifndef INCLUDE_MAIN_SB_VM_H
#define INCLUDE_MAIN_SB_VM_H

/*
 * Registers a native owner: appends its methods (a table ending with a null name) to sbNativeMethodTable and
 * the owner to sbNativeOwners. Returns 0 when there are 128 owners or 1024 methods already.
 */
int sbNativeOwnerAdd(const char *name, SbNativeMethod *methods);

#endif /* INCLUDE_MAIN_SB_VM_H */
