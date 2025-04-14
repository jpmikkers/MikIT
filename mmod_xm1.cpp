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

File:           MMOD_XM1.CPP
Description:    -
Version:        1.00 - original
                1.01 - don't check number of envelope points to see if an instrument header is corrupted
				1.02 - added extra check to sample loading: don't load samples of zero length
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mmod_xm.h"


MMODULE_XM::XMENV::XMENV()
{
    envelope=NULL;
    flag=0;
}


MMODULE_XM::XMENV::~XMENV()
{
    delete[] envelope;
}


SWORD MMODULE_XM::XMENV::Interpolate(SWORD p,SWORD p1,SWORD p2,SWORD v1,SWORD v2)
{
    SWORD dp,dv,di;

    if(p1==p2) return v1;

    dv=v2-v1;
    dp=p2-p1;
    di=p-p1;

    return v1 + ((SLONG)(di*dv) / dp);
}


void MMODULE_XM::XMENV::Expand(XMENVTUPLE tuples[12],UBYTE numtuples,UBYTE iflag,UBYTE isus,UBYTE ibeg,UBYTE iend)
{
    int t,p;
    SWORD p1,p2,v1,v2;

    flag=iflag;

    /* disable volume envelope if it doesn't have enough points */

    if((flag & XE_ON) && (numtuples<2)) flag&=~XE_ON;

	if(flag & XE_ON){
   		
		sus=tuples[isus].pos;
		beg=tuples[ibeg].pos;
		end=tuples[iend].pos;
		
		/* limit number of envelope points */
        if(numtuples>12) numtuples=12;

//#ifdef XMDEBUG
//		printf("%d points\n",numtuples);
//		for(t=0;t<12;t++) printf("(%u,%u) ",tuples[t].pos,tuples[t].val);
//		puts("");
//#endif

		points=1+tuples[numtuples-1].pos;

		envelope=new UBYTE[points];

		for(t=0;t<numtuples-1;t++){

			p1=tuples[t].pos;	v1=tuples[t].val;
			p2=tuples[t+1].pos;	v2=tuples[t+1].val;

			for(p=p1;p<p2;p++) envelope[p]=(UBYTE)Interpolate(p,p1,p2,v1,v2);
		}

		envelope[p2]=(UBYTE)v2;
	}
}



MMODULE_XM::XMSAMPLE::XMSAMPLE()
{
	handle=-1;
	msample=NULL;
}



MMODULE_XM::XMSAMPLE::~XMSAMPLE()
{
	delete msample;
//	if(handle>=0) driver->SampleFree(handle);
}


void MMODULE_XM::XMSAMPLE::LoadHeader(MINPUT *in)
{		
	bytelength		=in->read_ULONG();		/* (dword) Sample length */
	length			=bytelength;
	loopstart		=in->read_ULONG();      /* (dword) Sample loop start */
	loopend			=loopstart+in->read_ULONG();
	volume			=in->read_UBYTE();      /* (byte) Volume */
	finetune		=in->read_SBYTE();		/* (byte) Finetune (signed byte -128..+127) */
	UBYTE type		=in->read_UBYTE();      /* (byte) Type: Bit 0-1: 0 = No loop, 1 = Forward loop, */
											/* 2 = Ping-pong loop; */
											/* 4: 16-bit sampledata */
	panning			=in->read_UBYTE();		/* (byte) Panning (0-255) */
	relnote			=in->read_SBYTE();		/* (byte) Relative note number (signed byte) */
	in->read_UBYTE();						/* (byte) Reserved */
	in->read_UBYTES(samplename,22);			/* (char) Sample name */
//	printf("  - %22.22s\n",samplename);

	if(in->eof()) THROW MikModException("sample header corrupted");

	mmFlg=SF_DELTA|SF_SIGNED;

	switch(type&3){
		case 1:
			mmFlg|=SF_LOOP;
			break;
		case 2:
			mmFlg|=SF_LOOP;
			mmFlg|=SF_BIDI;
			break;
	}

	if(type & (1<<4)) mmFlg|=SF_16BITS;

    if(mmFlg&SF_16BITS){
		length>>=1;
        loopstart>>=1;
        loopend>>=1;
	}
}


void MMODULE_XM::XMSAMPLE::LoadBody(MINPUT *in)
{
	msample=new MSAMPLE(in,length,loopstart,loopend,mmFlg);
	if(in->eof()) THROW MikModException("sample body corrupted");
}


MMODULE_XM::XMINSTRUMENT::XMINSTRUMENT()
{
	samples=NULL;
}


MMODULE_XM::XMINSTRUMENT::~XMINSTRUMENT()
{
	delete[] samples;
}


void MMODULE_XM::XMINSTRUMENT::dummy()
{
	numsmp=0;
	memset(what,0,96);
	vibflg=0;
	vibsweep=0;
	vibdepth=0;
	vibrate=0;
	volfade=0;
}


void MMODULE_XM::XMINSTRUMENT::Connect(MDRIVER *d)
{
	// connect samples to the device
    
	for(int t=0;t<numsmp;t++){
		samples[t].handle=d->PrepareSample(samples[t].msample);
	}
}


void MMODULE_XM::XMINSTRUMENT::Disconnect(MDRIVER *d)
{
	// disconnect samples
    
	for(int t=0;t<numsmp;t++){
		d->UnPrepareSample(samples[t].handle);
		samples[t].handle=-1;
	}
}


void MMODULE_XM::XMINSTRUMENT::Load(MINPUT *in)
{
	int t;

	ULONG size=in->read_ULONG();		/* (dword) Instrument size */
	in->read_STRING(name,22);			/* (char) Instrument name */

//	printf("%22.22s\n",name);
	
	in->read_UBYTE();					/* (byte) Instrument type (always 0) */
	numsmp=in->read_UWORD();			/* (word) Number of samples in instrument */

	/* if it has samples, read instrument patch header */

//	printf("%d samples\n",numsmp);

	if(numsmp){
		XMENVTUPLE vtuples[12];
		XMENVTUPLE ptuples[12];

		in->read_ULONG();						/* (dword) headersize */
		in->read_UBYTES(what,96);				/* (byte) Sample number for all notes */

		for(t=0;t<12;t++){
			vtuples[t].pos=in->read_UWORD();
			vtuples[t].val=in->read_UWORD();
		}

		for(t=0;t<12;t++){
			ptuples[t].pos=in->read_UWORD();
			ptuples[t].val=in->read_UWORD();
		}

		UBYTE volpts		=in->read_UBYTE();          /* (byte) Number of volume points */
		UBYTE panpts		=in->read_UBYTE();          /* (byte) Number of panning points */
		UBYTE volsus		=in->read_UBYTE();          /* (byte) Volume sustain point */
		UBYTE volbeg		=in->read_UBYTE();          /* (byte) Volume loop start point */
		UBYTE volend		=in->read_UBYTE();          /* (byte) Volume loop end point */
		UBYTE pansus		=in->read_UBYTE();          /* (byte) Panning sustain point */
		UBYTE panbeg		=in->read_UBYTE();          /* (byte) Panning loop start point */
		UBYTE panend		=in->read_UBYTE();          /* (byte) Panning loop end point */
		UBYTE volflg		=in->read_UBYTE();          /* (byte) Volume type: bit 0: On; 1: Sustain; 2: Loop */
		UBYTE panflg		=in->read_UBYTE();          /* (byte) Panning type: bit 0: On; 1: Sustain; 2: Loop */

		vibflg		=in->read_UBYTE();          /* (byte) Vibrato type */
		vibsweep	=in->read_UBYTE();			/* (byte) Vibrato sweep */
		vibdepth	=in->read_UBYTE();			/* (byte) Vibrato depth */
		vibrate		=in->read_UBYTE();          /* (byte) Vibrato rate */
		volfade		=in->read_UWORD();          /* (word) Volume fadeout */
		in->read_UWORD();						/* (word) Reserved */

		/* header check */

		if(in->eof()) THROW MikModException("instrument header corrupted");
	
		/* skip extra bytes */

		in->seek(size-(XMINSTRUMENTHEADERSIZE+XMPATCHHEADERSIZE),SEEK_CUR);

		/* extra header check */

		if( (volflg & XE_ON) && volpts>12 ){
			THROW MikModException("too many volenv points - instrument header corrupted?");
		}

		if( (panflg & XE_ON) && panpts>12 ){
			THROW MikModException("too many panenv points - instrument header corrupted?");
		}
		
		/* create volume and panning envelopes */
		
		volenv.Expand(vtuples,volpts,volflg,volsus,volbeg,volend);
		panenv.Expand(ptuples,panpts,panflg,pansus,panbeg,panend);

		/* allocate samples */

		samples=new XMSAMPLE[numsmp];

		/* load samples */
			
		for(t=0;t<numsmp;t++) samples[t].LoadHeader(in);

		ULONG next=in->tell();

		for(t=0;t<numsmp;t++){
			samples[t].LoadBody(in);
			next+=samples[t].bytelength;	
			in->seek(next,SEEK_SET);
		}
	}
	else{
		in->seek(size-XMINSTRUMENTHEADERSIZE,SEEK_CUR);
	}
}



MMODULE_XM::XMPATTERN::XMPATTERN()
{
	data=NULL;
	index=NULL;
}



MMODULE_XM::XMPATTERN::~XMPATTERN()
{
	delete[] data;
	delete[] index;
}


void MMODULE_XM::XMPATTERN::dummy()
{
	int t;
	/* allocate pattern index table */
	numrows=64;
    index=new UWORD[numrows];
	data=new UBYTE[64];

	for(t=0;t<32;t++){
		data[t]=0x80;
	}

	for(t=0;t<numrows;t++){
		index[t]=0;
	}
}


void MMODULE_XM::XMPATTERN::Load(MINPUT *in,UWORD numchn)
{
	ULONG headersize=in->read_ULONG();		/* (dword) Pattern header length */
	in->read_UBYTE();						/* (byte) Packing type (always 0) */
	numrows=in->read_UWORD();				/* (word) Number of rows in pattern (1..256) */
	UWORD packsize=in->read_UWORD();		/* (word) Packed patterndata size */

//	printf("%d rows\n",numrows);

	/* skip extra bytes */

	in->seek(headersize-XMPATTERNHEADERSIZE,SEEK_CUR);
	
	/* read pattern data, if any */

	if(packsize>0){
		int row,chn;
		UWORD idx;

		/* allocate pattern data buffer */

		data=new UBYTE[packsize];

		/* read the data */

		in->read_UBYTES(data,packsize);

		/* allocate pattern index table */

        index=new UWORD[numrows];

		/* build index table */

		for(idx=0,row=0;row<numrows;row++){

			if(idx>65530) THROW MikModException("Pattern too big");

			index[row]=idx;

			for(chn=0;chn<numchn;chn++){

				UBYTE flag=data[idx];

				if(flag & 0x80){
					idx++;
					if(flag&1) idx++;
					if(flag&2) idx++;
					if(flag&4) idx++;
					if(flag&8) idx++;
					if(flag&16) idx++;
				}
				else idx+=5;
			}
		}
	}
}

