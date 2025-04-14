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
DRV_IWC.C

Description:
Mikmod driver for output on interwave codec

*/
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <malloc.h>
#include <conio.h>
#include <string.h>
#include "mtypes.h"
#include "mdma.h"
#include "mirq.h"
#include "mdrv_fmx.h"
#include "mdrv_iwc.h"

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

#define _CMODEI           0x0C        /* index for CMODEI */
#define CODEC_MODE1       0x00
#define CODEC_MODE2       0x40
#define CODEC_MODE3       0x6C        /* Enhanced Mode */
#define TIMER_ENABLE      0x40        /* CFIG2I[6] */
#define _CLTIMI           0x14
#define _CUTIMI           0x15
#define _CFIG2I           0x10
#define _CSR3I            0x18        /* Index to CSR3I (Interrupt Status) */
#define _CEXTI            0x0A        /* Index to External Control Register */
#define _CFIG1I           0x09        /* Index to Codec Conf Reg 1 */
#define _CPDFI            0x08        /* Index to Play Data Format Reg */
#define _CRDFI            0x1C        /* Index to Rec Data Format Reg */
#define DMA_ACCESS        0x00
#define PIO_ACCESS        0xC0
#define DMA_SIMPLEX       0x04
#define XTAL1             0x00        /* CxDFI[4]=0 selects 24.5Mhz XTAL */
#define XTAL2             0x01        /* CxDFI[4]=1 selects 16.9Mhz XTAL */

/************************************************************************/
/* Indirect register definitions for CODEC_ADDR register */
/* Bits 0-3. Select an internal register to read/write */
#define LEFT_ADC_INPUT      0x00    /* Left input control register */
#define RIGHT_ADC_INPUT     0x01    /* RIght input control register */
#define LEFT_AUX1_INPUT     0x02    /* Left Aux #1 input control */
#define RIGHT_AUX1_INPUT    0x03    /* Right Aux #1 input control */
#define LEFT_SYNTH_INPUT    0x02    /* Left Aux #1 input control */
#define RIGHT_SYNTH_INPUT   0x03    /* Right Aux #1 input control */
#define LEFT_AUX2_INPUT     0x04    /* Left Aux #2 input control */
#define RIGHT_AUX2_INPUT    0x05    /* Right Aux #2 input control */
#define LEFT_CD_INPUT       0x04    /* Left Aux #2 input control */
#define RIGHT_CD_INPUT      0x05    /* Right Aux #2 input control */
#define LEFT_DAC_OUTPUT     0x06    /* Left output control */
#define RIGHT_DAC_OUTPUT    0x07    /* Right output control */
#define PLAYBACK_FORMAT     0x08    /* Clock and data format */
#define CONFIG_1            0x09    /* Interface control */
#define EXTERNAL_CONTROL    0x0a    /* Pin control */
#define STATUS_2            0x0b    /* Test and initialization */
#define MODE_SELECT_ID      0x0c    /* Miscellaneaous information */
#define LOOPBACK            0x0d    /* Digital Mix */
#define UPPER_PLAY_COUNT    0x0e    /* Playback Upper Base Count */
#define LOWER_PLAY_COUNT    0x0f    /* Playback Lower Base Count */
#define CONFIG_2            0x10    /* alternate #1 feature enable */
#define CONFIG_3            0x11    /* alternate #2 feature enable */
#define LEFT_LINE_INPUT     0x12    /* left line input control */
#define RIGHT_LINE_INPUT    0x13    /* right line input control */
#define UPPER_TIMER         0x14    /* timer low byte */
#define LOWER_TIMER         0x15    /* timer high byte */
#define LEFT_MIC_INPUT      0x16
#define RIGHT_MIC_INPUT     0x17
#define STATUS_3            0x18    /* irq status register */
#define LEFT_MASTER_OUTPUT  0x19    /* irq status register */
#define MONO_IO_CTRL        0x1a    /* mono input/output control */
#define RIGHT_MASTER_OUTPUT 0x1b    /* irq status register */
#define RECORD_FORMAT       0x1c    /* record format */
#define PLAYBACK_FREQ       0x1d
#define UPPER_RECORD_COUNT  0x1e    /* record upper count */
#define LOWER_RECORD_COUNT  0x1f    /* record lower count */

#define DIG_AUDIO_MASTER    0x20
#define DIG_MUSIC_MASTER    0x21
#define MIDI_MASTER         0x22
#define EFFECTS_MASTER      0x23
#define EFFECTS_MUTE        0x24
#define DIG_AUDIO_MUTE      0x25
#define DIG_MUSIC_MUTE      0x26
#define MIDI_MUTE       0x27

/************************************************************************/
/* Control bit definitions for CODEC_ADDR register */
#define CODEC_INIT  0x80        /* CODEC is initializing */
#define CODEC_MCE   0x40        /* Mode change enable */
#define CODEC_DTD   0x20        /* Transfer Request Disable */
/************************************************************************/

/************************************************************************/
/* Definitions for CODEC_STATUS (STATUS_1) register */
#define CODEC_RULB  0x80        /* Capture data upper/lower byte */
#define CODEC_RLR   0x40        /* Capture left/right sample */
#define CODDEC_RDA  0x20        /* Capture data read */
#define CODEC_SE    0x10        /* Playback over/under run error */
#define CODEC_PULB  0x08        /* Playback upper/lower byte */
#define CODEC_PLR   0x04        /* Playback left/right sample */
#define CODEC_PBA   0x02        /* Playback data register read */
#define CODEC_GINT  0x01        /* interrupt status */
/************************************************************************/

/************************************************************************/
/* Definitions for STATUS_2 register                                    */
#define CODEC_STATUS2_RFO 0x80  /* Record FIFO overrun */
#define CODEC_STATUS2_PFU 0x40  /* Playback FIFO underrun */
#define CODEC_CACT        0x20  /* Calibration Active */
#define CODEC_DMA_ACTIVE  0x10  /* DMA Request line status */
#define CODEC_RADO_MASK   0x0C  /* Right Overrange Detect */
#define CODEC_LADO_MASK   0x03  /* Left Overrange Detect */
/************************************************************************/

/************************************************************************/
/* Definitions for STATUS_3 register                                    */
#define CODEC_TIR          0x40   /* Timer Interrupt Request */
#define CODEC_RFDI         0x20   /* Record FIFO Interrupt Request */
#define CODEC_PFDI         0x10   /* Playback FIFO Interrupt Request */
#define CODEC_RFU          0x08   /* Record FIFO underrun */
#define CODEC_STATUS3_RFO  0x04   /* Record FIFO overrun */
#define CODEC_PFO          0x02   /* Playback FIFO overrun */
#define CODEC_STATUS3_PFU  0x01   /* Playback FIFO underrun */
#define CODEC_STAT_MASK    0x70
/************************************************************************/

/************************************************************************/
/* Definitions for LEFT_ADC_INPUT and RIGHT_ADC_INPUT register */
#define SOURCE_LINE   0x00      /* Line source selected */
#define SOURCE_AUX1   0x40      /* Auxiliary 1 source selected */
#define SOURCE_MIC    0x80      /* Microphone source selected */
#define SOURCE_MIXER      0xC0      /* Post-mixed DAC output selected */
/************************************************************************/

/************************************************************************/
/* Mute enable for AUX1, AUX2, LINE, MIC, DAC, MASTER Control registers */
#define CODEC_MUTE_ENABLE   0x80
/************************************************************************/

/************************************************************************/
/* Definition for LOOPBACK register                                     */
#define CODEC_LBE   0x01
/************************************************************************/

/************************************************************************/
/* Playback and record data formats                                     */
#define CODEC_STEREO                  0x10
#define CODEC_16BIT_SIGNED_BIG_END    0xC0
#define CODEC_IMA_ADPCM           0xA0
#define CODEC_8BIT_ALAW           0x60
#define CODEC_16BIT_SIGNED_LITTLE_END 0x40
#define CODEC_8BIT_ULAW           0x20
#define CODEC_8BIT_UNSIGNED       0x00

/* Clock Divisor             Sample Rate in KHz  
                              (24.576 MHz / 16.9344 MHz ) */
#define CODEC_CLOCK_0 0x00   /*  8.0      / 5.51   */
#define CODEC_CLOCK_1 0x02   /*  16.0     / 11.025 */
#define CODEC_CLOCK_2 0x04   /*  27.42    / 18.9   */
#define CODEC_CLOCK_3 0x06   /*  32.0     / 22.05  */
#define CODEC_CLOCK_4 0x08   /*           / 37.8   */
#define CODEC_CLOCK_5 0x0A   /*           / 44.1   */
#define CODEC_CLOCK_6 0x0C   /*  48.0     / 33.075 */
#define CODEC_CLOCK_7 0x0E   /*  9.6      / 6.62   */

/* Crystal Select */
#define CODEC_XTAL2         0x01    /* 16.9344 crystal */
#define CODEC_XTAL1         0x00    /* 24.576 crystal */
/************************************************************************/

/************************************************************************/
/* Definitions for CONFIG_1 register                                    */
#define CODEC_CFIG1I_DEFAULT    0x00
#define CODEC_CAPTURE_PIO   0x80    /* Capture PIO enable */
#define CODEC_PLAYBACK_PIO  0x40    /* Playback PIO enable */
#define CODEC_AUTOCALIB     0x08    /* auto calibrate */
#define CODEC_SINGLE_DMA    0x04    /* Use single DMA channel */
#define CODEC_RE        0x02    /* Capture enable */
#define CODEC_PE        0x01    /* playback enable */
/************************************************************************/

/************************************************************************/
/* Definitions for CONFIG_2 register                                    */
#define CODEC_CFIG2I_DEFAULT    0x81
#define CODEC_OFVS 0x80  /* Output Full Scale Voltage */
#define CODEC_TE   0x40  /* Timer Enable */
#define CODEC_RSCD 0x20  /* Recors Sample Counter Disable */
#define CODEC_PSCD 0x10  /* Playback Sample Counter Disable */
#define CODEC_DAOF 0x01  /* D/A Ouput Force Enable */
/************************************************************************/

/************************************************************************/
/* Definitions for CONFIG_3 register                                    */
#define CODEC_CFIG3I_DEFAULT    0xC2 // 0x02 when synth DACs are working
#define CODEC_RPIE    0x80  /* Record FIFO IRQ Enable */
#define CODEC_PPIE    0x40  /* Playback FIFO IRQ Enable */
#define CODEC_FT_MASK 0x30  /* FIFO Threshold Select */
#define CODEC_PVFM    0x04  /* Playback Variable Frequency Mode */
#define CODEC_SYNA    0x02  /* AUX1/Synth Signal Select */
/************************************************************************/

/************************************************************************/
/* Definitions for EXTERNAL_CONTROL register                            */  
#define CODEC_CEXTI_DEFAULT     0x00
#define CODEC_IRQ_ENABLE    0x02    /* interrupt enable */
#define CODEC_GPOUT1        0x80    /* external control #1 */
#define CODEC_GPOUT0        0x40    /* external control #0 */
/************************************************************************/

/************************************************************************/
/* Definitions for MODE_SELECT_ID register                              */
#define CODEC_MODE_DEFAULT 0x40
#define CODEC_MODE_MASK  0x60
#define CODEC_ID_BIT4    0x80
#define CODEC_ID_BIT3_0  0x0F
/************************************************************************/


MDRIVER_IWC::MDRIVER_IWC() : MDRIVER_FMX() , irqlnk(this)
{
}


MDRIVER_IWC::~MDRIVER_IWC()
{
}

/*
Implementation of IRQLINK, derived from MHWINTERRUPT. This is just a 'glue'
class which knows what handler to call when the interrupt is active.
This is also the reason why IRQLINK has to be a friend class of MDRIVER_IWC
*/

MDRIVER_IWC::IRQLINK::IRQLINK(MDRIVER_IWC *p)
{
        parent=p;
}


void MDRIVER_IWC::IRQLINK::handle()
{
        parent->handleirq();        
        EOI();        
}


/***************************************************************************
>>>>>>>>>>>>>>>>>>>>>>>>> Lowlevel IW stuff <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
***************************************************************************/


int MDRIVER_IWC::getprofilestring(char *filename,char *section,char *entry,char *buffer)
{
    FILE *f;
    char buf[256],*p;

    /* open file */

    f=fopen(filename,"r");
    if(f==NULL) return 0;

    /* find section */

    while(1){

        if(!fgets(buf,255,f)){
            fclose(f);
            return 0;
        }

        // skip whitespace
        p=buf; while(*p==' ' || *p=='\t') p++;

        // found section?
        if(*p=='[' && !strnicmp(p+1,section,strlen(section))) break;
    }

    /* find entry */

    while(1){

        if(!fgets(buf,255,f)){
            fclose(f);
            return 0;
        }

        // skip whitespace
        p=buf; while(*p==' ' || *p=='\t') p++;

        // skip comments
        if(*p==';') continue;

        // new section? -> didn't find it
		if(*p=='['){
			fclose(f);
			return 0;
		}

		if( !strnicmp(p,entry,strlen(entry))){
			p+=strlen(entry);
			
			while(*p==' ' || *p=='\t') p++;

			if(*p=='='){
				p++;
				while(*p!=';' && *p!='\n' && *p!=0) *buffer++=*p++;					
				*buffer=0;
				fclose(f);
				return 1;
			}
		}
	}

	fclose(f);
	return 1;
}



int MDRIVER_IWC::Ping()
/*
        Checks if a codec is present at the current baseport

        returns: TRUE   => codec is present
                         FALSE  => No codec detected
*/
{
//	printf("Found codec revision %d at port %x\n",CodecRevision(),pcodar);
	CodecRevision();
	return ((inportb(pcodar)&0x1f)==MODE_SELECT_ID);
}


void MDRIVER_IWC::handleirq()
{
	if(inportb(pcodar+0x02)&CODEC_GINT){
		UBYTE saveindex=inportb(pcodar);		// save current index register
		
		UBYTE source=CodecRead(_CSR3I);

		if(source & CODEC_TIR){					// Timer Interrupt Request ?
			/* ... */
			CodecWrite(_CSR3I,CODEC_RFDI|CODEC_PFDI);
		}

		if(source & CODEC_RFDI){				// Record FIFO Interrupt Request ?
			/* ... */
			CodecWrite(_CSR3I,CODEC_PFDI|CODEC_TIR);
		}

		if(source & CODEC_PFDI){				// Playback FIFO Interrupt Request ?
			/* ... */
			interruptcounter++;
			CodecWrite(_CSR3I,CODEC_RFDI|CODEC_TIR);
		}

		outportb(pcodar,saveindex);				// restore index register
	}
}
	


int MDRIVER_IWC::IsThere()
{
	char dir[256];
   	char *envptr,*endptr,buffer[256];

	p2xr	=0xffff;
	pcodar	=0xffff;
	cdatap	=0xffff;
    irq		=0xff;
    lodma	=0xff;
    hidma	=0xff;
    
	if((envptr=getenv("INTERWAVE"))==NULL){
		if((envptr=getenv("IWDIR"))==NULL) return 0;
		strcpy(dir,envptr);
		strcat(dir,"\\IW.INI");
	}
	else{
		strcpy(dir,envptr);
	}

	if(!getprofilestring(dir,"setup 0","synthbase",buffer)) return 0;

	p2xr=strtol(buffer,&endptr,16);
	
	if(!getprofilestring(dir,"setup 0","codecbase",buffer)) return 0;
	
	pcodar=strtol(buffer,&endptr,16);
	cdatap=pcodar+1;

	if(!getprofilestring(dir,"setup 0","irq1",buffer)) return 0;

	irq=strtol(buffer,&endptr,10);

	if(getprofilestring(dir,"setup 0","dma1",buffer)){
		lodma=strtol(buffer,&endptr,10);
	}

	if(!getprofilestring(dir,"setup 0","dma2",buffer)) return 0;	// dma 2 is required
	
	hidma=strtol(buffer,&endptr,10);
	
	if(!Ping()) return 0;
	return 1;
}


UBYTE MDRIVER_IWC::CodecRead(UBYTE index)
{
	outportb(pcodar,(inportb(pcodar)&0xE0)|index);
	return inportb(cdatap);
}


void MDRIVER_IWC::CodecWrite(UBYTE index,UBYTE value)
{
	outportb(pcodar,(inportb(pcodar)&0xE0)|index);
	outportb(cdatap,value);
}


void MDRIVER_IWC::CodecMode(UBYTE mode)
{
	CodecWrite(_CMODEI,mode);
}


UBYTE MDRIVER_IWC::CodecRevision()
{
	UBYTE result=CodecRead(MODE_SELECT_ID);
	result=((result&0x80)>>3)+(result&0xf);
	return result;
}


void MDRIVER_IWC::CodecTimerStart()
{
	CodecWrite(_CFIG2I,CodecRead(_CFIG2I)|TIMER_ENABLE);
}


void MDRIVER_IWC::CodecTimerStop()
{
	CodecWrite(_CFIG2I,CodecRead(_CFIG2I)&~TIMER_ENABLE);
}


void MDRIVER_IWC::CodecSetTimer(UWORD cnt)
{
	CodecWrite(_CUTIMI,cnt>>8);
	CodecWrite(_CLTIMI,cnt&0xff);
}



UWORD MDRIVER_IWC::CodecGetTimer()
{
	return (UWORD)(CodecRead(_CUTIMI)<<8)|CodecRead(_CLTIMI);
}


void MDRIVER_IWC::CodecEnableIrq()
{
	CodecWrite(_CEXTI,CodecRead(_CEXTI)|CODEC_IRQ_ENABLE);		// global codec interrupt enable
}


void MDRIVER_IWC::CodecDisableIrq()
{
	CodecWrite(_CEXTI,CodecRead(_CEXTI)&~CODEC_IRQ_ENABLE);		// disable codec interrupts
}


void MDRIVER_IWC::CodecClearIrq()
{
	CodecWrite(_CSR3I,0);										// clear all interrupts
}


void MDRIVER_IWC::CodecPlayAccess(UBYTE type)
{
	UBYTE reg;
	
	reg=inportb(pcodar)&0xE0;
	outportb(pcodar,reg|_CFIG1I|0x40);
 
	if(type==PIO_ACCESS)            // configure CFIG1I
		outportb(cdatap,inportb(cdatap)|0x40);
	else
		outportb(cdatap,(inportb(cdatap)&0xBF)|type);
	outportb(pcodar,reg|_CFIG1I);    // write protect CFIG1I[7:2]
}


void MDRIVER_IWC::CodecPlayDataFormat(UBYTE data)
{
	UBYTE reg=inportb(pcodar)&0xE0;
//  if (iw.cmode==CODEC_MODE1)  inx=_CPDFI;
	outportb(pcodar,reg|_CPDFI|CODEC_MCE);		// turn CIDXR[MCE] on/ select register
	outportb(cdatap,data);						// set data format
	outportb(pcodar,reg|_CPDFI);				// turn CIDXR[MCE] off
}


void MDRIVER_IWC::CodecPlayFrequency(UWORD freq)
{
	UBYTE var, reg;

	struct{
		UBYTE freq,bits;
	} codec_freq[14] = {
		{5, 0x00|XTAL2},{6, 0x0E|XTAL2},
		{8, 0x00|XTAL1},{9, 0x0E|XTAL1},
		{11,0x02|XTAL2},{16,0x02|XTAL1},
		{18,0x04|XTAL2},{22,0x06|XTAL2},
		{27,0x04|XTAL1},{32,0x06|XTAL1},
		{33,0x0C|XTAL2},{37,0x08|XTAL2},
		{44,0x0A|XTAL2},{48,0x0C|XTAL1}
	};

	freq=freq/1000;
	if(freq>48) freq=48;

	for(var=0; var<14; var++){         /* select closest frequency */
		if (freq<=codec_freq[var].freq)	break;
	}

	reg=inportb(pcodar)&0xE0;
	outportb(pcodar,reg|_CPDFI|0x40);
	var=(inportb(cdatap)&0xF0)|codec_freq[var].bits;
	outportb(cdatap,var);
	outportb(pcodar,reg|_CPDFI);			/* write protect the registers */
}


void MDRIVER_IWC::CodecPlayCount(UWORD cnt)
{
	cnt--;
	CodecWrite(UPPER_PLAY_COUNT,cnt>>8);
	CodecWrite(LOWER_PLAY_COUNT,cnt&0xff);
}


void MDRIVER_IWC::CodecPlayStart()
{
	CodecWrite(_CFIG1I,CodecRead(_CFIG1I)|CODEC_PE);	/* playback enable */
}


void MDRIVER_IWC::CodecPlayStop()
{
	CodecWrite(_CFIG1I,CodecRead(_CFIG1I)&~CODEC_PE);	/* playback enable */
}


void MDRIVER_IWC::CodecDisableOutput()
{
	outportb(p2xr,inportb(p2xr)|0x02);       /* disable output */
}


void MDRIVER_IWC::CodecEnableOutput()
{
	outportb(p2xr,inportb(p2xr)&0xfd);       /* enable output */
}


/***************************************************************************
>>>>>>>>>>>>>>>>>>>>>>>>>> Hilevel SB stuff <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
***************************************************************************/


void MDRIVER_IWC::Init()
{
    if(IsThere()){
        printf("IW codec at port %x, irq %d, lodma %d, hidma %d\n",pcodar,irq,lodma,hidma);
    }
    else{
        THROW MikModException("No Interwave codec detected");
    }

    irqlnk.Install(irq);

	CodecMode(CODEC_MODE3);			// set codec to mode 3
	CodecPlayAccess(DMA_ACCESS);	// use DMA
//	CodecPlayAccess(DMA_SIMPLEX);	// use DMA

	dmabuffersize=latency;
    dmabuffersize*=frequency;
    if(mode&DMODE_STEREO) dmabuffersize<<=1;
    if(mode&DMODE_16BITS) dmabuffersize<<=1;
    dmabuffersize/=1000;
    dmabuffersize=(dmabuffersize+0xf)&0xfffffff0;

    printf("Allocating %d byte dmabuffer\n",dmabuffersize);

    dma.Init(hidma,
             dmabuffersize,
             MDMA::INDEF_WRITE);

    MDRIVER_FMX::Init();
}                 


void MDRIVER_IWC::Exit()
{
	irqlnk.Disable();
    irqlnk.Remove();
    dma.Stop();
    dma.Exit();
    MDRIVER_FMX::Exit();
}


void MDRIVER_IWC::Start()
{
	dmalast=0;

    irqlnk.Enable();
	CodecClearIrq();
//        CodecEnableIrq();

	CodecPlayDataFormat(CODEC_STEREO|CODEC_16BIT_SIGNED_LITTLE_END);
	CodecPlayFrequency(44100);
	CodecPlayCount(16384);

	WriteSilentBytes((SBYTE *)dma.GetPtr(),dmabuffersize);

    dma.Start();
	
	interruptcounter=0;
	
	// enable output and play
	CodecEnableOutput();
	CodecPlayStart();

    MDRIVER_FMX::Start();
}


void MDRIVER_IWC::Stop()
{
	CodecDisableOutput();
	CodecPlayStop();
	dma.Stop();
//        CodecDisableIrq();
	irqlnk.Disable();
        MDRIVER_FMX::Stop();
}


void MDRIVER_IWC::Update()
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

