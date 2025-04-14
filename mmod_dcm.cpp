/*

File:			MMOD_DCM.CPP
Description:	-
Version:		1.00 - original

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mmod_dcm.h"

/*
        Command 1 flags:
*/
#define F1_NOP  0x80    // no operation
#define F1_FRQ  0x40    // new frequency
#define F1_VOL  0x20    // new volume
#define F1_PAN  0x10    // new panpos
#define F1_INS  0x08    // new instrument
#define F1_OFS  0x04    // offset!=0
#define F1_KCK  0x02    // start sample
#define F1_CHN  0x01    // chain bit

/*
        Command 2 flags:
*/
#define F2_NOO  0x80    // note-off
#define F2_BPM  0x40    // new bpm
#define F2_TRG  0x20    // trigger bit
#define F2_CHN  0x01    // chain bit (unused)

int MMODULE_DCM::Test(MINPUT *i,char *title,int maxlen)
{
	UBYTE id[4];
    i->setmode(MINPUT::INTEL);
    i->rewind();
    if(!i->read_UBYTES(id,4)) return 0;
	return !memcmp(id,"DCM1",4);
}

int MMODULE_DCM::GetPos()
{
	return timestamp/1000;
}


void MMODULE_DCM::NextPos()
{
}


void MMODULE_DCM::PrevPos()
{
}


int MMODULE_DCM::GetNumChannels()
{
	return numchn;
}


int MMODULE_DCM::GetSongLength()
{
	return 1024;
}


int MMODULE_DCM::IsReady()
{
	return isready;
}


char *MMODULE_DCM::GetSongTitle()
{
	return "Unknown";
}


char *MMODULE_DCM::GetSampleName(int s)
{
	return "Unknown";
}


int MMODULE_DCM::GetNumSamples()
{
	return numsmp;	
}


MMODULE_DCM::MMODULE_DCM() : MMODULE()
{
    pattern=NULL;
	samples=NULL;
    isplaying=0;
}


void MMODULE_DCM::Load(MINPUT *i)
{
    UBYTE id[4];
    int t;
	in=i;

    moduletype="DCM1";
    
    /* try to read module header */
	i->setmode(MINPUT::INTEL);
    i->rewind();

	// read header id
	i->read_UBYTES(id,4);

	// check it
	if(memcmp(id,"DCM1",4)){
		THROW MikModException("Not a DCM lite file");
	}

	// read other header info
	numchn=i->read_UBYTE();
	numsmp=i->read_UBYTE();
	streamsize=i->read_ULONG();
	streamrepp=i->read_ULONG();

	//printf("numchn %d numsmp %d, streamsize %ld, streamrepp %ld\n",numchn,numsmp,streamsize,streamrepp);

	samples=new DCMSAMPLE[numsmp];

	for(t=0;t<numsmp;t++){
		samples[t].LoadHeader(in);
	}

	pattern=new UBYTE[streamsize];
	i->read_UBYTES(pattern,streamsize);

	if(i->eof()) THROW MikModException("Header too short");

	for(t=0;t<numsmp;t++)
	{
		samples[t].LoadBody(i);
	}

	pc=pattern;
	streamend=pattern+streamsize;
}



MMODULE_DCM::~MMODULE_DCM()
{
	Stop();
	delete[] pattern;
	pattern=NULL;
	delete[] samples;
	samples=NULL;
}


void MMODULE_DCM::Restart()
{
}


void MMODULE_DCM::Start(MDRIVER *d)
{
	int t;
	
	if(isplaying) return;

	driver=d;

	timestamp=0;

	pc = pattern;
	nopcount = 0;

	for(t=0;t<numchn;t++){
		voices[t]=driver->AllocVoice();
	}

	driver->ampfactor=(float)pow(driver->channels,0.52);
	driver->SetBPM(125);

	for(t=0;t<numsmp;t++){
		samples[t].handle=driver->PrepareSample(samples[t].msample);
	}

	for(t=0;t<numchn;t++){
		driver->VoiceSetPanning(voices[t],0x80);
	}

	isready=0;
	isplaying=1;
}


void MMODULE_DCM::Stop()
{
	int t;

    if(!isplaying) return;

	// free all voices
	for(t=0;t<numchn;t++){
		driver->FreeVoice(voices[t]);
	}

	// disconnect samples
	for(t=0;t<numsmp;t++){
		driver->UnPrepareSample(samples[t].handle);
		samples[t].handle=-1;
	}

	isplaying=0;
}


static ULONG DecodeFrequency(UWORD v)
{
	UBYTE exp = (UBYTE)(v>>14);
	ULONG frq = v&0x3fff;
	frq = (frq<<exp) + 0x4000L*((1<<exp)-1);
	return frq;
}


void MMODULE_DCM::Update()
{
	int t;
	UBYTE code;

	if(!isplaying || isready ) return;

	if(pc>=streamend){
		pc=pattern+streamrepp;

		if(!loopmode)
		{
			isready=1;
		}
	}

	// handle each channel

	for(t=0; t<numchn ;t++){

		int voice = voices[t];

		if(nopcount){		// we're busy doing nothing ?
			nopcount--;
		}
		else{               // no, we have to fetch a new dcm-lite command
			ULONG start;

			code=*pc++;

			// NO oPeration?

			if(code&F1_NOP){
				nopcount=code&0x7f;
				continue;
			}

			// Set frequency ?

			if(code&F1_FRQ){
				driver->VoiceSetFrequency(voice,DecodeFrequency((((UWORD)pc[1])<<8) + (UWORD)pc[0]));
				pc+=2;
			}

            // Set volume (0-255) ?
			if(code&F1_VOL) driver->VoiceSetVolume(voice,*pc++);

			// Set panning position (0-255) ?
			if(code&F1_PAN) driver->VoiceSetPanning(voice,*pc++);

			// Set instrument ?
			if(code&F1_INS) instrument[t]=*pc++;

			// Set Offset ?

			if(code&F1_OFS){
				start=*pc++;
				start<<=8;
			}
			else start=0;

			// Start sample?

			if(code&F1_KCK){
				UBYTE snum=instrument[t];		// samplenumber

				if( snum < numsmp )
				{
					DCMSAMPLE *sample = &samples[snum];
					driver->VoicePlay(voice, sample->handle, start, sample->length, sample->loopstart, sample->loopend, 0, 0, sample->flags);
				}
			}

            // chain bit set?

			if(code&F1_CHN){

                // yep, so read new code

				code=*pc++;

				// Note off ?
				if(code&F2_NOO) driver->VoiceStop(voice);

				// Set bpm rate (32-255) ?
				if(code&F2_BPM) driver->SetBPM(*pc++);

				// trigger fx ?
                if(code&F2_TRG){
                        UBYTE triggerdata=*pc++;

                        /* insert interesting trigger stuff
                            here :^) */
                }
			}
		}
	}

	timestamp+=(125L*1000)/(50L*driver->GetBPM());
}


int MMODULE_DCM::GetTimeStamp()
{
	return timestamp;
}


void MMODULE_DCM::SetTimeStamp(int ts)
{
	timestamp=ts;
}

MMODULE_DCM::DCMSAMPLE::DCMSAMPLE()
{
	handle=-1;
	msample=NULL;
}


MMODULE_DCM::DCMSAMPLE::~DCMSAMPLE()
{
	delete msample;
}


void MMODULE_DCM::DCMSAMPLE::LoadHeader(MINPUT *i)
{
    length=i->read_ULONG();
    loopstart=i->read_ULONG();
    loopend=i->read_ULONG();
    UWORD dcmflags=i->read_UWORD();
	id=i->read_UWORD();

    // here I have to convert the DCM sample flag to a mikit flag
	flags=0;
	if(dcmflags & 1) flags|=SF_16BITS;	// Sample is 16 bits (default 8 bit)
	if(dcmflags & 2) flags|=SF_SIGNED;  // Sample is signed (default unsigned)
	if(dcmflags & 4) flags|=SF_LOOP;	// Sample loops
	if(dcmflags & 8) flags|=SF_BIDI;	// Sample has BIDI loop

	//printf("Sample id=%d, length=%ld, loopstart=%ld, loopend=%ld, flags=%d\n",id,length,loopstart,loopend,flags);
}


void MMODULE_DCM::DCMSAMPLE::LoadBody(MINPUT *in)
{
	msample=new MSAMPLE(in,length,loopstart,loopend,flags);
	if(in->eof()) THROW MikModException("sample body corrupted");
}
