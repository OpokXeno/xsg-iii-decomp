#ifndef SCE_SIF_H
#define SCE_SIF_H

/* SIF RPC client side of the EE SDK (sifrpc.h). */

typedef struct sceSifRpcData {
    void *paddr;
    unsigned int pid;
    int tid;
    unsigned int mode;
} sceSifRpcData;

typedef void (*sceSifEndFunc)(void *parameter);

/* One RPC client: serve (+0x24) is set once sceSifBindRpc has reached the server. */
typedef struct sceSifClientData {
    sceSifRpcData rpcd;
    unsigned int command;  /* +0x10 */
    void *buff;            /* +0x14 */
    void *cbuff;           /* +0x18 */
    sceSifEndFunc func;    /* +0x1C */
    void *para;            /* +0x20 */
    struct sceSifServeData *serve; /* +0x24 */
} sceSifClientData;

int sceSifBindRpc(sceSifClientData *client, unsigned int request, unsigned int mode);
int sceSifCallRpc(sceSifClientData *client, unsigned int function, unsigned int mode, void *send, int sendSize,
                  void *receive, int receiveSize, sceSifEndFunc endFunc, void *endParameter);

#endif
