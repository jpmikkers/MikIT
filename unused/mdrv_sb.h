/*

*/
#ifndef MDRV_SB_H
#define MDRV_SB_H

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <malloc.h>
#include <conio.h>
#include "mtypes.h"
#include "mdma.h"
#include "mirq.h"
#include "mdrv_fmx.h"

class MDRIVER_SB : public MDRIVER_FMX {

        class IRQLINK : public MHWINTERRUPT {
                protected:
                MDRIVER_SB *parent;
                virtual void handle();
        
                public:
                IRQLINK(MDRIVER_SB *p);
        };

        friend IRQLINK;

private:
        IRQLINK irqlnk;
        MDMA    dma;

        UWORD   port;
        UWORD   dspversion;    /* DSP version number */
        UBYTE   irq;           /* sb irq */
        UBYTE   lodma;         /* 8 bit dma channel (1.0/2.0/pro) */
        UBYTE   hidma;         /* 16 bit dma channel (16/16asp) */

        UWORD   timeconstant;
        int     dmabuffersize;
        int     dmalast;

        void    handleirq();

        void    MixerStereo();
        void    MixerMono();
        int     WaitDSPWrite();
        int     WaitDSPRead();
        int     WriteDSP(UBYTE data);
        UWORD   ReadDSP();
        void    SpeakerOn();
        void    SpeakerOff();
        void    ResetDSP();
        int     Ping();
        UWORD   GetDSPVersion();

public:
        int     IsThere();
        void    Init();
        void    Exit();
        void    Start();
        void    Stop();
    void    Update();

        MDRIVER_SB();
        virtual ~MDRIVER_SB();
};


#endif
