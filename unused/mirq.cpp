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
Some general purpose IRQ routines

Portability:

MSDOS:  BC(y)   Watcom(y)   DJGPP(y)
Win95:  n
Os2:    n
Linux:  n

(y) - yes
(n) - no (not possible or not useful)
(?) - may be possible, but not tested

*/
#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <mem.h>
#include <string.h>
#include "mirq.h"

#define OCR1    0x20            /* 8259-1 Operation control register */
#define IMR1    0x21            /* 8259-1 Mask register */

#define OCR2    0xA0            /* 8259-2 Operation control register */
#define IMR2    0xA1            /* 8259-2 Mask register */

typedef void (*IRQHANDLER)(void *optr);

typedef void (interrupt *PVI)(void);

extern "C" {
extern void  MyStub(void);
extern ULONG MyVect1;
extern ULONG MyVect2;
}


MINTERRUPT::MINTERRUPT()
{
        thunk=new UBYTE[256];
        vecno=0;
        oldhandler=NULL;
}


MINTERRUPT::~MINTERRUPT()
{
        Remove();
        delete[] thunk;
}


void MINTERRUPT::Install(UWORD ivecno)
{
        if(IsInstalled()) return;

        MyVect1=(ULONG)this;
        MyVect2=(ULONG)irqhandler;
        memcpy(thunk,(void *)MyStub,256);

        vecno=ivecno;
        oldhandler=_dos_getvect(vecno);
        _dos_setvect(vecno,(PVI)thunk);
}


void MINTERRUPT::Remove()
{
        if(!IsInstalled()) return;
        _dos_setvect(vecno,oldhandler);
        vecno=0;
}


int MINTERRUPT::IsInstalled()
{
        return (vecno!=0);
}


void MINTERRUPT::chain()
{
        if(oldhandler) oldhandler();
}


static void _saveregs _loadds MINTERRUPT::irqhandler(MINTERRUPT *t)
{
        t->handle();
}


MHWINTERRUPT::MHWINTERRUPT() : MINTERRUPT()
{
}


MHWINTERRUPT::~MHWINTERRUPT()
{
}


void MHWINTERRUPT::Install(UBYTE iirq)
{
        if(IsInstalled()) return;
        irq=iirq;
        UWORD vecno=(irq>7) ? irq+0x68 : irq+0x8;
        MINTERRUPT::Install(vecno);
}


void MHWINTERRUPT::EOI()
{
        outportb(0x20,0x20);
        if(irq>7) outportb(0xa0,0x20);
}


int MHWINTERRUPT::IsEnabled(UBYTE irqno)
{
        UBYTE imr=(irqno>7) ? IMR2 : IMR1;              /* interrupt mask register */
        UBYTE msk=1<<(irqno&7);                         /* interrupt mask */

        if(irqno>7) if(!IsEnabled(2)) return 0;
        return ((inportb(imr) & msk) == 0);
}


int MHWINTERRUPT::IsEnabled()
{
        return(IsEnabled(irq));
}


void MHWINTERRUPT::OnOff(UBYTE irqno,UBYTE onoff)
/*
    Use to enable or disable the specified irq.
*/
{
    UBYTE imr=(irqno>7) ? IMR2 : IMR1;      /* interrupt mask register */
    UBYTE ocr=(irqno>7) ? OCR2 : OCR1;      /* ocr */
        UBYTE msk=1<<(irqno&7);                         /* interrupt mask */
        UBYTE eoi=0x60|(irqno&7);                       /* specific end-of-interrupt */

    if(onoff){
        outportb(imr,inportb(imr) & ~msk);
        outportb(ocr,eoi);
                if(irqno>7) OnOff(2,1);
    }
    else{
        outportb(imr,inportb(imr) | msk);
    }
}


void MHWINTERRUPT::Enable()
{
        OnOff(irq,1);
}


void MHWINTERRUPT::Disable()
{
        OnOff(irq,0);
}

#ifdef NEVER

class MTIMER : public MHWINTERRUPT {
        protected:
        virtual void handle();

        public:
        volatile int teller;

        MTIMER();
        ~MTIMER();
};


MTIMER::MTIMER() : MHWINTERRUPT()
{
        teller=0;        
}


MTIMER::~MTIMER()
{
}


void MTIMER::handle()
{
//        puts("Hello");
        teller++;
        chain();
//        EOI();        
}


void main()
{
        MTIMER t;

        t.Install(0);
        t.Enable();

        while(!kbhit()){
                printf("%d\n",t.teller);
                delay(10);
        }

        t.Remove();
}
#endif
