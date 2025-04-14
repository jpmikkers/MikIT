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

Name:
MDMA.C

Description:
DMA routines

*/
#include <dos.h>
#include <malloc.h>
#include <conio.h>
#include "mtypes.h"
#include "mdma.h"


/* DMA Controler #1 (8-bit controller) */
#define DMA1_STAT       0x08            /* read status register */
#define DMA1_WCMD       0x08            /* write command register */
#define DMA1_WREQ       0x09            /* write request register */
#define DMA1_SNGL       0x0A            /* write single bit register */
#define DMA1_MODE       0x0B            /* write mode register */
#define DMA1_CLRFF      0x0C            /* clear byte ptr flip/flop */
#define DMA1_MCLR       0x0D            /* master clear register */
#define DMA1_CLRM       0x0E            /* clear mask register */
#define DMA1_WRTALL     0x0F            /* write all mask register */

/* DMA Controler #2 (16-bit controller) */
#define DMA2_STAT       0xD0            /* read status register */
#define DMA2_WCMD       0xD0            /* write command register */
#define DMA2_WREQ       0xD2            /* write request register */
#define DMA2_SNGL       0xD4            /* write single bit register */
#define DMA2_MODE       0xD6            /* write mode register */
#define DMA2_CLRFF      0xD8            /* clear byte ptr flip/flop */
#define DMA2_MCLR       0xDA            /* master clear register */
#define DMA2_CLRM       0xDC            /* clear mask register */
#define DMA2_WRTALL     0xDE            /* write all mask register */

#define DMA0_ADDR       0x00            /* chan 0 base adddress */
#define DMA0_CNT        0x01            /* chan 0 base count */
#define DMA1_ADDR       0x02            /* chan 1 base adddress */
#define DMA1_CNT        0x03            /* chan 1 base count */
#define DMA2_ADDR       0x04            /* chan 2 base adddress */
#define DMA2_CNT        0x05            /* chan 2 base count */
#define DMA3_ADDR       0x06            /* chan 3 base adddress */
#define DMA3_CNT        0x07            /* chan 3 base count */
#define DMA4_ADDR       0xC0            /* chan 4 base adddress */
#define DMA4_CNT        0xC2            /* chan 4 base count */
#define DMA5_ADDR       0xC4            /* chan 5 base adddress */
#define DMA5_CNT        0xC6            /* chan 5 base count */
#define DMA6_ADDR       0xC8            /* chan 6 base adddress */
#define DMA6_CNT        0xCA            /* chan 6 base count */
#define DMA7_ADDR       0xCC            /* chan 7 base adddress */
#define DMA7_CNT        0xCE            /* chan 7 base count */

#define DMA0_PAGE       0x87            /* chan 0 page register (refresh)*/
#define DMA1_PAGE       0x83            /* chan 1 page register */
#define DMA2_PAGE       0x81            /* chan 2 page register */
#define DMA3_PAGE       0x82            /* chan 3 page register */
#define DMA4_PAGE       0x8F            /* chan 4 page register (unuseable)*/
#define DMA5_PAGE       0x8B            /* chan 5 page register */
#define DMA6_PAGE       0x89            /* chan 6 page register */
#define DMA7_PAGE       0x8A            /* chan 7 page register */

#define MAX_DMA         8

#define DMA_DECREMENT   0x20    /* mask to make DMA hardware go backwards */

typedef struct {
    UBYTE dma_disable;      /* bits to disable dma channel */
    UBYTE dma_enable;       /* bits to enable dma channel */
    UWORD page;                     /* page port location */
    UWORD addr;                     /* addr port location */
    UWORD count;            /* count port location */
    UWORD single;           /* single mode port location */
    UWORD mode;                     /* mode port location */
    UWORD clear_ff;         /* clear flip-flop port location */
    UBYTE write;            /* bits for write transfer */
    UBYTE read;                     /* bits for read transfer */
} DMA_ENTRY;



/* Variables needed ... */

static DMA_ENTRY mydma[MAX_DMA] = {

/* DMA channel 0 */
            {0x04,0x00,DMA0_PAGE,DMA0_ADDR,DMA0_CNT,
             DMA1_SNGL,DMA1_MODE,DMA1_CLRFF,0x48,0x44},

/* DMA channel 1 */
            {0x05,0x01,DMA1_PAGE,DMA1_ADDR,DMA1_CNT,
             DMA1_SNGL,DMA1_MODE,DMA1_CLRFF,0x49,0x45},

/* DMA channel 2 */
            {0x06,0x02,DMA2_PAGE,DMA2_ADDR,DMA2_CNT,
             DMA1_SNGL,DMA1_MODE,DMA1_CLRFF,0x4A,0x46},

/* DMA channel 3 */
            {0x07,0x03,DMA3_PAGE,DMA3_ADDR,DMA3_CNT,
             DMA1_SNGL,DMA1_MODE,DMA1_CLRFF,0x4B,0x47},

/* DMA channel 4 */
            {0x04,0x00,DMA4_PAGE,DMA4_ADDR,DMA4_CNT,
             DMA2_SNGL,DMA2_MODE,DMA2_CLRFF,0x48,0x44},

/* DMA channel 5 */
            {0x05,0x01,DMA5_PAGE,DMA5_ADDR,DMA5_CNT,
             DMA2_SNGL,DMA2_MODE,DMA2_CLRFF,0x49,0x45},

/* DMA channel 6 */
            {0x06,0x02,DMA6_PAGE,DMA6_ADDR,DMA6_CNT,
             DMA2_SNGL,DMA2_MODE,DMA2_CLRFF,0x4A,0x46},

/* DMA channel 7 */
            {0x07,0x03,DMA7_PAGE,DMA7_ADDR,DMA7_CNT,
                         DMA2_SNGL,DMA2_MODE,DMA2_CLRFF,0x4B,0x47}
};

#define ENTER_CRITICAL IRQ_PUSH_OFF()
extern void IRQ_PUSH_OFF (void);
#pragma aux IRQ_PUSH_OFF =      \
    "pushfd",                   \
        "cli"           \
                modify [esp];

#define LEAVE_CRITICAL IRQ_POP()
extern void IRQ_POP (void);
#pragma aux IRQ_POP =   \
        "popfd"         \
                modify [esp];


MDMA::MDMA()
{
        running=0;
        didinit=0;
}


int MDMA::Init(UBYTE ichannel,int iblocksize,UBYTE imode)
{
        channel=ichannel;
        blocksize=iblocksize;
        mode=imode;

        dma_disable     =mydma[channel].dma_disable;
        dma_enable      =mydma[channel].dma_enable;
        write           =mydma[channel].write;
        read            =mydma[channel].read;
        port_page       =mydma[channel].page;
        port_addr       =mydma[channel].addr;
        port_count      =mydma[channel].count;
        port_single     =mydma[channel].single;
        port_mode       =mydma[channel].mode;
        port_clear_ff   =mydma[channel].clear_ff;

        union REGS r;
    ULONG p;

    /* allocate TWICE the size of the requested dma buffer..
    this fixes the 'page-crossing' bug of previous versions */

    r.x.eax = 0x0100;           /* DPMI allocate DOS memory */
        r.x.ebx = ((blocksize*2) + 15) >> 4; /* Number of paragraphs requested */

    int386 (0x31, &r, &r);

        if( r.x.cflag ) return 0;         /* failed */

        dma_selector=r.x.edx;

    /* convert the segment into a linear address */

    p=(r.x.eax&0xffff)<<4;

    /* if the first half of the allocated memory crosses a page
    boundary, return the second half which is then guaranteed to
    be page-continuous */

        if( (p>>16) != ((p+blocksize-1)>>16) ) p+=blocksize;

        dma_continuous=(void *)p;

        didinit=1;
        return 1;
}


void MDMA::Exit()
{
        if(!didinit) return;
        union REGS r;
        r.x.eax = 0x0101;               /* DPMI free DOS memory */
        r.x.edx = dma_selector;         /* base selector */
    int386 (0x31, &r, &r);
        didinit=0;
}


MDMA::~MDMA()
{
}


void MDMA::Start()
{
        ULONG s_20bit,e_20bit;
    UWORD spage,saddr,tcount;
    UWORD epage,eaddr;
    UBYTE cur_mode;
        ULONG size;

        size=blocksize;

    /* Convert the pc address to a 20 bit physical
       address that the DMA controller needs */

        s_20bit = (ULONG)dma_continuous;
        e_20bit = s_20bit + size - 1;
    spage = s_20bit>>16;
    epage = e_20bit>>16;

        if(spage != epage) return;

    if(channel>=4){
        /* if 16-bit xfer, then addr,count & size are divided by 2 */
        s_20bit = s_20bit >> 1;
        e_20bit = e_20bit >> 1;
        size = size >> 1;
    }

    saddr=s_20bit&0xffff;

    tcount = size-1;

        switch(mode){

        case READ_DMA:
                        cur_mode = read;
            break;

        case WRITE_DMA:
                        cur_mode = write;
            break;

        case INDEF_READ:
                        cur_mode = read | 0x10;   /* turn on auto init */
            break;

        case INDEF_WRITE:
                        cur_mode = write | 0x10;  /* turn on auto init */
            break;
    }

    ENTER_CRITICAL;
        outportb(port_single,dma_disable);      /* disable channel */
        outportb(port_mode,cur_mode);           /* set mode */
        outportb(port_clear_ff,0);              /* clear f/f */
        outportb(port_addr,saddr&0xff);         /* LSB */
        outportb(port_addr,saddr>>8);           /* MSB */
        outportb(port_page,spage);              /* page # */
        outportb(port_clear_ff,0);              /* clear f/f */
        outportb(port_count,tcount&0x0ff);      /* LSB count */
        outportb(port_count,tcount>>8);         /* MSB count */
        outportb(port_single,dma_enable);       /* enable */
    LEAVE_CRITICAL;
        running=1;
}


void MDMA::Stop()
{
        if(!running) return;
        outportb(port_single,dma_disable);   /* disable chan */
        running=0;
}


ULONG MDMA::Todo()
{
        if(!running) return 0;

        ULONG result;
    UWORD val1,val2;

    ENTER_CRITICAL;

        outportb(port_clear_ff,0xff);

    redo:
        val1=inportb(port_count);
        val1|=inportb(port_count)<<8;
        val2=inportb(port_count);
        val2|=inportb(port_count)<<8;

    val1-=val2;
    if((SWORD)val1>64) goto redo;
    if((SWORD)val1<-64) goto redo;

    LEAVE_CRITICAL;

        result=val2;                     
        if(channel>3) result<<=1;

        return result;
}


void *MDMA::GetPtr()
{
        return(dma_continuous);
}


void MDMA::Commit(int index,int count)
{
}

