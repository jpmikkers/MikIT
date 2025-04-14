/*

(c) Copyright 1996, 1997 Jean-Paul Mikkers 

This file is part of MikIT.

MikIT is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 of 
the License, or (at your option) any later version.

MikIT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public 
License along with MikIT. If not, see <http://www.gnu.org/licenses/>.

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
