/*

*/
#ifndef MDMA_H
#define MDMA_H

#include "mtypes.h"

class MDMA {
        private:

        UBYTE channel,mode;
        int blocksize;
        int running;
        int didinit;

    UBYTE dma_disable;      /* bits to disable dma channel */
    UBYTE dma_enable;       /* bits to enable dma channel */
        UBYTE write;            /* bits for write transfer */
        UBYTE read;             /* bits for read transfer */
        UWORD port_page;        /* page port location */
        UWORD port_addr;        /* addr port location */
        UWORD port_count;       /* count port location */
        UWORD port_single;      /* single mode port location */
        UWORD port_mode;        /* mode port location */
        UWORD port_clear_ff;    /* clear flip-flop port location */

        ULONG dma_selector;
        void *dma_continuous;

        public:

        enum{
                READ_DMA,
                WRITE_DMA,
                INDEF_READ,
                INDEF_WRITE
        };

        MDMA();
        ~MDMA();

        int  Init(UBYTE ichannel,int iblocksize,UBYTE imode);
        void Exit();

        void Start();
        void Stop();

        ULONG Todo();

        void *GetPtr();
        void Commit(int index,int count);
};

#endif
