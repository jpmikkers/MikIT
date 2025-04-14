/*

File:           MDRV_FMX.CPP
Description:    -
Version:        1.00 - original
                1.01 - allow fasttracker style panning using SF_FTPAN flag
                1.02 - implemented two-pole resonant filtering
                1.03 - changed FRACBITS back from 16 to 14 (it caused clipping in the interpolations)
                1.02 - v0.91 fixed bug : increase low frequency sanity check to prevent zero division (keith 303 module)
                1.03 - v0.91 fixed bug : allocate twice as much memory when loading a 16 bit sample PLEASE
                1.04 - v0.92 fixed bug : on a noteoff, when going from a sustain loop to a normal loop, make a sample go forward
                1.05 - v1.00b2 don't load samples of zero length
				1.06 - v1.00b2 dynamic volume calculation based on uservolume
*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <assert.h>
#include "mdrv_scn.h"



MDRIVER_SCN::MDRIVER_SCN() : MDRIVER()
{
}



MDRIVER_SCN::~MDRIVER_SCN()
{
}



void MDRIVER_SCN::Init()
{
	maxlevel=0.0;
}



void MDRIVER_SCN::Exit()
{
}



void MDRIVER_SCN::Start()
{
	samplesthatfit=TICKBUFFERSIZE;
	if(mode & DMODE_STEREO) samplesthatfit>>=1;
	if(mode & DMODE_NOCLICK) mode|=DMODE_INTERP;
	tickleft=0;
	mixposition=0;
}



void MDRIVER_SCN::Stop()
{
}



void MDRIVER_SCN::SetBPM(UBYTE ibpm)
{
	bpm=ibpm;
}



UBYTE MDRIVER_SCN::GetBPM()
{
	return bpm;
}



int MDRIVER_SCN::AllocVoice()
{
	return -1;
}



void MDRIVER_SCN::FreeVoice(int handle)
{
}



int	MDRIVER_SCN::PrepareSample(MSAMPLE *s)
{
	return -1;
}



void MDRIVER_SCN::UnPrepareSample(int handle)
{
}



int MDRIVER_SCN::samples2bytes(int v)
{
	if(mode & DMODE_16BITS) v<<=1;
	if(mode & DMODE_STEREO) v<<=1;
	return v;
}



int MDRIVER_SCN::bytes2samples(int v)
{
	if(mode & DMODE_16BITS) v>>=1;
	if(mode & DMODE_STEREO) v>>=1;
	return v;
}




void MDRIVER_SCN::WriteSamples(SBYTE *buffer,int todo)
{
	int part;

	while(todo>0){
		if(tickleft==0){
			Tick();
			tickleft=(125L*frequency)/(50L*bpm);
		}
		part=min(tickleft,todo);
		part=min(part,samplesthatfit);
		tickleft-=part;
		todo-=part;
		mixposition+=part;
	}
}


ULONG MDRIVER_SCN::GetMixPosition()
{
	return mixposition;
}


int MDRIVER_SCN::WriteBytes(SBYTE *buffer,int todo)
{
	todo=bytes2samples(todo);
	WriteSamples(buffer,todo);
	return samples2bytes(todo);
}


void MDRIVER_SCN::WriteSilentSamples(SBYTE *buffer,int todo)
{
}


int MDRIVER_SCN::WriteSilentBytes(SBYTE *buffer,int todo)
{
	return todo;
}


void MDRIVER_SCN::VoiceSetVolume(int voice,UBYTE volume)
{	
}



void MDRIVER_SCN::VoiceSetPanning(int voice,UBYTE panning)
{	
}



void MDRIVER_SCN::VoiceSetFrequency(int voice,ULONG frequency)
{	
}



void MDRIVER_SCN::VoiceSetFilter(int voice,UBYTE cutoff,UBYTE damping)
{	
}



void MDRIVER_SCN::VoiceStop(int voice)
{	
}



int MDRIVER_SCN::VoiceActive(int voice)
{	
	return 0;
}



void MDRIVER_SCN::VoiceKeyOff(int voice)
{
}



void MDRIVER_SCN::VoiceKeyOn(int voice)
{
}



void MDRIVER_SCN::VoicePlay(int voice,int handle,ULONG start,ULONG size,ULONG reppos,ULONG repend,ULONG sreppos,ULONG srepend,UWORD flags)
{
}


void MDRIVER_SCN::Update()
{
	WriteBytes(NULL,4096);	
}
