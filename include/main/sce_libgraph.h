#ifndef INCLUDE_MAIN_SCE_LIBGRAPH_H
#define INCLUDE_MAIN_SCE_LIBGRAPH_H

/* libgraph: installs the VSync callback (0 removes it) and returns the previous one. */
int sceGsSyncVCallback(int (*callback)(void));

#endif /* INCLUDE_MAIN_SCE_LIBGRAPH_H */
