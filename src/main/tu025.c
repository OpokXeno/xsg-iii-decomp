#include "common.h"
#include "main/sce_sif.h"

typedef int s32;

extern sceSifClientData gIoRpcCallBlock;
extern s32 gIoRpcInputBlock[];
extern s32 gIoRpcReplyBlock[];
/* The reply word of the rpc block at gIoRpcReplyBlock. */
#define ioRpcReply (gIoRpcReplyBlock[0])

s32 ioRpcCall(s32 command) {
    return sceSifCallRpc(&gIoRpcCallBlock, command, 0, gIoRpcInputBlock, 0x40, gIoRpcReplyBlock, 0x40, 0, 0);
}

s32 ioRpcQueryValue(s32 queryValue) {
    gIoRpcInputBlock[0] = queryValue;
    ioRpcCall(0x100);
    return ioRpcReply;
}

s32 ioRpcQueryIndexed(s32 queryIndex) {
    gIoRpcInputBlock[0] = queryIndex;
    ioRpcCall(queryIndex + 0x200);
    return ioRpcReply;
}

void ioRpcSendNotify(void) {
    ioRpcCall(0x2FF);
}

s32 func_002E71E0();

typedef struct IoRpcPostedRecord {
    s32 firstOperand;                          /* +0x00 */
    s32 rpcReply;                         /* +0x04 */
    s32 secondOperand;                         /* +0x08 */
    s32 reserved;                       /* +0x0C, always 0 */
} IoRpcPostedRecord;

/* ioRpcCall returns the result of the rpc call (sceSifCallRpc). */
s32 ioRpcPostRecord(s32 command, s32 firstOperand, s32 secondOperand) {
    IoRpcPostedRecord postedRecord;
    s32 postResult;

    ioRpcCall(command + 0x400);
    postResult = -1;
    if (ioRpcReply != 0) {
        postedRecord.firstOperand = firstOperand;
        postedRecord.rpcReply = ioRpcReply;
        postedRecord.secondOperand = secondOperand;
        postedRecord.reserved = 0;
        func_002E71E0(&postedRecord.firstOperand, 1);
        postResult = ioRpcReply;
    }
    return postResult;
}

s32 func_002E7C30();

s32 ioRpcFetchBlock(s32 command, s32 firstOperand, s32 secondOperand) {
    s32 replyBlock[8];

    ioRpcCall(command + 0x500);
    if (ioRpcReply == 0) {
        return -1;
    }
    func_002E7C30(&replyBlock[0], ioRpcReply, firstOperand, secondOperand, 0);
    return ioRpcReply;
}

/* The input words of the rpc calls (query operands). */
s32 gIoRpcInputBlock[16];

/* The reply words of the rpc calls; ioRpcReply reads word 0. */
s32 gIoRpcReplyBlock[16];
