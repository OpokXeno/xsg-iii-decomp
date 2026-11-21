#include "common.h"

typedef int s32;

extern s32 gIoRpcCallBlock;
extern s32 gIoRpcInputBlock;
extern s32 gIoRpcReplyBlock[];
/* The reply word of the rpc block at gIoRpcReplyBlock. */
#define ioRpcReply (gIoRpcReplyBlock[0])

s32 func_002E80D8(s32 *rpc, s32 command, s32 extra, s32 *input, s32 inputSize, s32 *output, s32 outputSize, s32 flags, s32 option);

s32 ioRpcCall(s32 command) {
    return func_002E80D8(&gIoRpcCallBlock, command, 0, &gIoRpcInputBlock, 0x40, gIoRpcReplyBlock, 0x40, 0, 0);
}

s32 ioRpcQueryValue(s32 queryValue) {
    gIoRpcInputBlock = queryValue;
    ioRpcCall(0x100);
    return ioRpcReply;
}

s32 ioRpcQueryIndexed(s32 queryIndex) {
    gIoRpcInputBlock = queryIndex;
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

/* ioRpcCall returns the result of the rpc call (func_002E80D8). */
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
