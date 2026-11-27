#ifndef INCLUDE_MAIN_XGL_DMA_H
#define INCLUDE_MAIN_XGL_DMA_H

void xglDmaDirectNormal(unsigned int channel, unsigned int memoryAddress, unsigned int quadwordCount);
/* Starts a source-chain DMA on the channel from the DMA tag at tagAddress. */
void xglDmaDirectSrcChain(unsigned int channel, unsigned int tagAddress);

#endif /* INCLUDE_MAIN_XGL_DMA_H */
