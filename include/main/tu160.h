#ifndef INCLUDE_MAIN_TU160_H
#define INCLUDE_MAIN_TU160_H

typedef void (*CompletionCallback)(void);

/* Appends a completionCallback to the completion list and keeps it null-terminated; a null completionCallback clears the list. */
void completionCallbackAdd(CompletionCallback completionCallback);

#endif /* INCLUDE_MAIN_TU160_H */
