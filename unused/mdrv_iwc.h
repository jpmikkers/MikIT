/*

*/
#ifndef MDRV_IWC_H
#define MDRV_IWC_H

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <malloc.h>
#include <conio.h>
#include "mtypes.h"
#include "mdma.h"
#include "mirq.h"
#include "mdrv_fmx.h"

class MDRIVER_IWC : public MDRIVER_FMX {

        class IRQLINK : public MHWINTERRUPT {
                protected:
                MDRIVER_IWC *parent;
                virtual void handle();
        
                public:
                IRQLINK(MDRIVER_IWC *p);
        };

        friend IRQLINK;

private:
        IRQLINK irqlnk;
        MDMA    dma;

//      UWORD   port;
        UWORD   p2xr;
        UWORD   pcodar;
        UWORD   cdatap;

//      UWORD   dspversion;    /* DSP version number */
        UBYTE   irq;           /* sb irq */
        UBYTE   lodma;         /* 8 bit dma channel (1.0/2.0/pro) */
        UBYTE   hidma;         /* 16 bit dma channel (16/16asp) */

//      UWORD   timeconstant;
        int     dmabuffersize;
        int     dmalast;

        int     interruptcounter;

        void    handleirq();
    
        int     getprofilestring(char *filename,char *section,char *entry,char *buffer);

        UBYTE   CodecRead(UBYTE index);
        void    CodecWrite(UBYTE index,UBYTE value);
        void    CodecMode(UBYTE mode);
        UBYTE   CodecRevision();
        void    CodecTimerStart();
        void    CodecTimerStop();
        void    CodecSetTimer(UWORD cnt);
        UWORD   CodecGetTimer();
        void    CodecEnableIrq();
        void    CodecDisableIrq();
        void    CodecClearIrq();
        void    CodecPlayAccess(UBYTE type);
        void    CodecPlayDataFormat(UBYTE data);
        void    CodecPlayFrequency(UWORD freq);
        void    CodecPlayCount(UWORD cnt);
        void    CodecPlayStart();
        void    CodecPlayStop();
        void    CodecDisableOutput();
        void    CodecEnableOutput();

//      void    MixerStereo();
//      void    MixerMono();
//      int     WaitDSPWrite();
//      int     WaitDSPRead();
//      int     WriteDSP(UBYTE data);
//      UWORD   ReadDSP();
//      void    SpeakerOn();
//      void    SpeakerOff();
//      void    ResetDSP();
        int     Ping();
//      UWORD   GetDSPVersion();

public:
        int     IsThere();
        void    Init();
        void    Exit();
        void    Start();
        void    Stop();
    void    Update();

        MDRIVER_IWC();
        virtual ~MDRIVER_IWC();
};


#endif
