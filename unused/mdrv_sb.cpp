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
DRV_SB.C

Description:
Mikmod driver for output on Creative Labs Soundblasters & compatibles
(through DSP)

*/
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <malloc.h>
#include <conio.h>
#include "mtypes.h"
#include "mdma.h"
#include "mirq.h"
#include "mdrv_fmx.h"
#include "mdrv_sb.h"

/*
    Define some important SB i/o ports:
*/

#define MIXER_ADDRESS           (port+0x4)
#define MIXER_DATA              (port+0x5)
#define DSP_RESET               (port+0x6)
#define DSP_READ_DATA           (port+0xa)
#define DSP_WRITE_DATA          (port+0xc)
#define DSP_WRITE_STATUS        (port+0xc)
#define DSP_DATA_AVAIL          (port+0xe)


MDRIVER_SB::MDRIVER_SB() : MDRIVER_FMX() , irqlnk(this)
{
}


MDRIVER_SB::~MDRIVER_SB()
{
}

/*
Implementation of IRQLINK, derived from MHWINTERRUPT. This is just a 'glue'
class which knows what handler to call when the interrupt is active.
This is also the reason why IRQLINK has to be a friend class of MDRIVER_SB.
*/

MDRIVER_SB::IRQLINK::IRQLINK(MDRIVER_SB *p)
{
        parent=p;
}


void MDRIVER_SB::IRQLINK::handle()
{
        parent->handleirq();        
        EOI();        
}

/***************************************************************************
>>>>>>>>>>>>>>>>>>>>>>>>> Lowlevel SB stuff <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
***************************************************************************/


void MDRIVER_SB::MixerStereo()
/*
    Enables stereo output for DSP versions 3.00 >= ver < 4.00
*/
{
    outportb(MIXER_ADDRESS,0xe);
    outportb(MIXER_DATA,inportb(MIXER_DATA)|2);
}



void MDRIVER_SB::MixerMono()
/*
    Disables stereo output for DSP versions 3.00 >= ver < 4.00
*/
{
    outportb(MIXER_ADDRESS,0xe);
    outportb(MIXER_DATA,inportb(MIXER_DATA)&0xfd);
}



int MDRIVER_SB::WaitDSPWrite()
/*
    Waits until the DSP is ready to be written to.

    returns FALSE on timeout
*/
{
    UWORD timeout=32767;

    while(timeout--){
        if(!(inportb(DSP_WRITE_STATUS)&0x80)) return 1;
    }
    return 0;
}



int MDRIVER_SB::WaitDSPRead()
/*
    Waits until the DSP is ready to read from.

    returns FALSE on timeout
*/
{
    UWORD timeout=32767;

    while(timeout--){
        if(inportb(DSP_DATA_AVAIL)&0x80) return 1;
    }
    return 0;
}



int MDRIVER_SB::WriteDSP(UBYTE data)
/*
    Writes byte 'data' to the DSP.

    returns FALSE on timeout.
*/
{
        if(!WaitDSPWrite()) return 0;
    outportb(DSP_WRITE_DATA,data);
    return 1;
}



UWORD MDRIVER_SB::ReadDSP()
/*
    Reads a byte from the DSP.

    returns 0xffff on timeout.
*/
{
        if(!WaitDSPRead()) return 0xffff;
    return(inportb(DSP_READ_DATA));
}



void MDRIVER_SB::SpeakerOn()
/*
    Enables DAC speaker output.
*/
{
        WriteDSP(0xd1);
}



void MDRIVER_SB::SpeakerOff()
/*
    Disables DAC speaker output
*/
{
        WriteDSP(0xd3);
}



void MDRIVER_SB::ResetDSP()
/*
    Resets the DSP.
*/
{
    int t;
    /* reset the DSP by sending 1, (delay), then 0 */
    outportb(DSP_RESET,1);
    for(t=0;t<8;t++) inportb(DSP_RESET);
    outportb(DSP_RESET,0);
}



int MDRIVER_SB::Ping()
/*
    Checks if a SB is present at the current baseport by
    resetting the DSP and checking if it returned the value 0xaa.

    returns: TRUE   => SB is present
             FALSE  => No SB detected
*/
{
        ResetDSP();
        return(ReadDSP()==0xaa);
}


void MDRIVER_SB::handleirq()
{
        if(dspversion<0x200){
                WriteDSP(0x14);
                WriteDSP(0xff);
                WriteDSP(0xfe);
    }

        if(mode & DMODE_16BITS)
                inportb(port+0xf);
        else
                inportb(DSP_DATA_AVAIL);
}


UWORD MDRIVER_SB::GetDSPVersion()
/*
    Gets SB-dsp version. returns 0xffff if dsp didn't respond.
*/
{
	UWORD hi,lo;

        if(!WriteDSP(0xe1)) return 0xffff;

        hi=ReadDSP();
        lo=ReadDSP();

	return((hi<<8)|lo);
}


int MDRIVER_SB::IsThere()
{
	char *envptr,c;
        char *endptr;

        port =0xffff;
        irq  =0xff;
        lodma=0xff;
        hidma=0xff;

	if((envptr=getenv("BLASTER"))==NULL) return 0;

	while(1){

		/* skip whitespace */

		do c=*(envptr++); while(c==' ' || c=='\t');

		/* reached end of string? -> exit */

		if(c==0) break;

		switch(c){

			case 'a':
			case 'A':
                                port=strtol(envptr,&endptr,16);
				break;

			case 'i':
			case 'I':
                                irq=strtol(envptr,&endptr,10);
				break;

			case 'd':
			case 'D':
                                lodma=strtol(envptr,&endptr,10);
				break;

			case 'h':
			case 'H':
                                hidma=strtol(envptr,&endptr,10);
				break;

			default:
				strtol(envptr,&endptr,16);
				break;
		}
		envptr=endptr;
	}

        if(port==0xffff || irq==0xff || lodma==0xff) return 0;

        if(!Ping()) return 0;

	/* get dsp version. */

        if((dspversion=GetDSPVersion())==0xffff) return 0;

	return 1;
}

/***************************************************************************
>>>>>>>>>>>>>>>>>>>>>>>>>> Hilevel SB stuff <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
***************************************************************************/


void MDRIVER_SB::Init()
{
        if(IsThere()){
                printf("SB version %x at port %x, irq %d, lodma %d, hidma %d\n",dspversion,port,irq,lodma,hidma);
        }
        else{
                THROW MikModException("No SB detected");
        }

        if(dspversion>=0x400 && hidma==0xff){
                THROW MikModException("High-dma setting in 'BLASTER' variable is required for SB-16");
	}

        if(dspversion<0x400 && (mode&DMODE_16BITS)){
                /* DSP versions below 4.00 can't do 16 bit sound. */
                mode&=~DMODE_16BITS;
        }

        if(dspversion<0x300 && (mode&DMODE_STEREO)){
        /* DSP versions below 3.00 can't do stereo sound. */
                mode&=~DMODE_STEREO;
	}

        if(dspversion<0x400){
                int t=frequency;
                if(mode & DMODE_STEREO) t<<=1;

                timeconstant=256-(1000000L/t);

                if(dspversion<0x201){
                        if(timeconstant>210) timeconstant=210;
		}
		else{
                        if(timeconstant>233) timeconstant=233;
		}

                frequency=1000000L/(256-timeconstant);
                if(mode & DMODE_STEREO) frequency>>=1;
	}

        irqlnk.Install(irq);
//        irqlnk.Enable();

        dmabuffersize=latency;
        dmabuffersize*=frequency;
        if(mode&DMODE_STEREO) dmabuffersize<<=1;
        if(mode&DMODE_16BITS) dmabuffersize<<=1;
        dmabuffersize/=1000;
        dmabuffersize=(dmabuffersize+0xf)&0xfffffff0;

        printf("Allocating %d byte dmabuffer\n",dmabuffersize);

        dma.Init((mode & DMODE_16BITS) ? hidma : lodma,
                 dmabuffersize,
                 MDMA::INDEF_WRITE);

        MDRIVER_FMX::Init();
}                 


void MDRIVER_SB::Exit()
{
        irqlnk.Disable();
        irqlnk.Remove();
        dma.Stop();
        dma.Exit();
        MDRIVER_FMX::Exit();
}


void MDRIVER_SB::Start()
{
        dmalast=0;

        irqlnk.Enable();

        if(dspversion>=0x300 && dspversion<0x400){
                if(mode&DMODE_STEREO){
                        MixerStereo();
                }
                else{
                        MixerMono();
                }
        }

        WriteSilentBytes((SBYTE *)dma.GetPtr(),dmabuffersize);

        dma.Start();

        if(dspversion<0x400){

                SpeakerOn();

                WriteDSP(0x40);
                WriteDSP(timeconstant);

                if(dspversion<0x200){
                        WriteDSP(0x14);
                        WriteDSP(0xff);
                        WriteDSP(0xfe);
		}
                else if(dspversion==0x200){
                        WriteDSP(0x48);
                        WriteDSP(0xff);
                        WriteDSP(0xfe);
                        WriteDSP(0x1c);
		}
		else{
                        WriteDSP(0x48);
                        WriteDSP(0xff);
                        WriteDSP(0xfe);
                        WriteDSP(0x90);
		}
        }
        else{
                WriteDSP(0x41);
                WriteDSP(frequency>>8);
                WriteDSP(frequency&0xff);
        
                if(mode&DMODE_16BITS){
                        WriteDSP(0xb6);
                        WriteDSP(mode&DMODE_STEREO ? 0x30 : 0x10);
                }
                else{
                        WriteDSP(0xc6);
                        WriteDSP(mode&DMODE_STEREO ? 0x20 : 0x00);
                }

                WriteDSP(0xff);
                WriteDSP(0xef);
        }

        MDRIVER_FMX::Start();
}


void MDRIVER_SB::Stop()
{
        SpeakerOff();
        ResetDSP();
        ResetDSP();
        dma.Stop();
        irqlnk.Disable();
        MDRIVER_FMX::Stop();
}


void MDRIVER_SB::Update()
{
        int dmacur,todo;
        SBYTE *p;

        p=(SBYTE *)dma.GetPtr();

        dmacur=(dmabuffersize-dma.Todo())&0xffff0;

        if(dmacur==dmalast) return;

        if(dmacur>dmalast){
                todo=dmacur-dmalast;
                WriteBytes(p+dmalast,todo);
	}
        else{
                todo=dmabuffersize-dmalast;
                WriteBytes(p+dmalast,todo);
                WriteBytes(p,dmacur);
	}

        dmalast=dmacur;
}


