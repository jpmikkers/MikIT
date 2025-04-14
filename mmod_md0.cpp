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

File:           MMOD_MD0.CPP
Description:    -
Version:        1.00 - original

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mmod_mod.h"


const int MODULEHEADERSIZE=1084;



MMODULE_MOD::MODTYPE MMODULE_MOD::modtypes[22] = {
       { "M.K.",4  },       /* protracker 4 channel */
       { "M!K!",4  },       /* protracker 4 channel */
       { "FLT4",4  },       /* startracker 4 channel */
       { "4CHN",4  },       /* fasttracker 4 channel */
       { "6CHN",6  },       /* fasttracker 6 channel */
       { "8CHN",8  },       /* fasttracker 8 channel */
       { "10CH",10 },       /* fasttracker 8 channel */
       { "12CH",12 },       /* fasttracker 8 channel */
       { "14CH",14 },       /* fasttracker 8 channel */
       { "16CH",16 },       /* fasttracker 8 channel */
       { "18CH",18 },       /* fasttracker 8 channel */
       { "20CH",20 },       /* fasttracker 8 channel */
       { "22CH",22 },       /* fasttracker 8 channel */
       { "24CH",24 },       /* fasttracker 8 channel */
       { "26CH",26 },       /* fasttracker 8 channel */
       { "28CH",28 },       /* fasttracker 8 channel */
       { "30CH",30 },       /* fasttracker 8 channel */
       { "32CH",32 },       /* fasttracker 8 channel */
       { "CD81",8  },       /* atari oktalyzer 8 channel */
       { "OKTA",8  },       /* atari oktalyzer 8 channel */
       { "16CN",16 },       /* taketracker 16 channel */
       { "32CN",32 }        /* taketracker 32 channel */
};        



int MMODULE_MOD::Test(MINPUT *i,char *title,int maxlen)
{
    UBYTE id[4];

    i->setmode(MINPUT::MOTOROLA);
    i->rewind();
    i->seek(MODULEHEADERSIZE-4,SEEK_SET);
    if(!i->read_UBYTES(id,4)) return 0;

    /* find out which ID string */

    for(int t=0;t<22;t++){
            if(!memcmp(id,modtypes[t].id,4)) return 1;
    }

    return 0;
}


int MMODULE_MOD::GetPos()
{
    if(!isplaying) return 0;
    return songpos;
}


void MMODULE_MOD::NextPos()
{
    if(!isplaying) return;
    for(int t=0;t<numchn;t++) driver->VoiceStop(audio[t].voice);
    posjump=songpos+2;
}


void MMODULE_MOD::PrevPos()
{
    if(!isplaying) return;
    if(songpos){
        for(int t=0;t<numchn;t++) driver->VoiceStop(audio[t].voice);
        posjump=songpos;
    }
}


int MMODULE_MOD::GetNumChannels()
{
    return numchn;
}


int MMODULE_MOD::GetSongLength()
{
    return songlength;
}


int MMODULE_MOD::IsReady()
{
    return isready;
}


char *MMODULE_MOD::GetSongTitle()
{
    return songname;
}


char *MMODULE_MOD::GetSampleName(int s)
{
    return samples[s].name;
}


int MMODULE_MOD::GetNumSamples()
{
    return numsmp;  
}


MMODULE_MOD::MMODULE_MOD() : MMODULE()
{
    /* hier members initialiseren */

    patterns=NULL;
    samples=NULL;
    isplaying=0;
}


void MMODULE_MOD::Load(MINPUT *i)
{
    int t;
    in=i;

    moduletype="Protracker";
    
    /* try to read module header */

    i->setmode(MINPUT::MOTOROLA);
    i->rewind();
    i->read_STRING(songname,20);

    numsmp=31;
    
    /* allocate sample array */

    samples=new MODSAMPLE[numsmp];

    for(t=0;t<numsmp;t++){
        samples[t].LoadHeader(in);
    }
    
    songlength  = i->read_UBYTE();
    UBYTE magic1= i->read_UBYTE();

    i->read_UBYTES(positions,128);

    UBYTE magic2[4];
    
    i->read_UBYTES(magic2,4);

    if(i->eof()) THROW MikModException("Header too short");

    // identify this module

    for(t=0;t<22;t++){
        if(!memcmp(magic2,modtypes[t].id,4)) break;
    }

    if(t==22) THROW MikModException("Unknown type of module");

    // get number of channels

    numchn=modtypes[t].channels;

    // Count the number of patterns

    numpat=0;

    for(t=0;t<128;t++){             /* check ALL positions */
        if(positions[t]>numpat) numpat=positions[t];
    }

    numpat++;

    if(infomode) return;                // don't load other stuff in info mode
	
//      numtrk=numpat*numchn;

	// allocate pattern array
    
	patterns=new MODPATTERN[numpat];

	// load each pattern

    for(t=0;t<numpat;t++){
		patterns[t].Load(i,numchn);
		TerminateThrow();
	}                        

	// load each sample

	for(t=0;t<numsmp;t++){
		samples[t].LoadBody(i);
		TerminateThrow();
	}
}



MMODULE_MOD::~MMODULE_MOD()
{
	Stop();
	delete[] patterns;
	patterns=NULL;
	delete[] samples;
	samples=NULL;
}


void MMODULE_MOD::Restart()
{
	int t;

	isplaying=0;
	timestamp=0;
	speed=6;
    tick=speed;
	bpm=125;
	breakpos=0;
	posjump=0;
	patdly=0;
	patdly2=0;
	loopcnt=0;
	looppos=0;
	songpos=0;
	row=-1;

	for(t=0;t<numchn;t++){
		int v=audio[t].voice;
		memset(&audio[t],0,sizeof(AUDTMP));
		audio[t].voice=v;
		audio[t].panning=((t+1)&2) ? 0 : 0xff;
		audio[t].parent=this;
	}

	driver->SetBPM(bpm);
	isready=0;
	isplaying=1;
}



void MMODULE_MOD::Start(MDRIVER *d)
{
	int t;
	
	if(isplaying) return;

	driver=d;

	timestamp=0;

	speed=6;
    tick=speed;
	bpm=125;

//	pattern=&patterns[0];

	breakpos=0;
	posjump=0;
	patdly=0;
	patdly2=0;
	loopcnt=0;
	looppos=0;

	songpos=0;
	row=-1;

	for(t=0;t<numchn;t++){
		memset(&audio[t],0,sizeof(AUDTMP));
		audio[t].voice=driver->AllocVoice();
		audio[t].panning=((t+1)&2) ? 0 : 0xff;
		audio[t].parent=this;
	}

	driver->SetBPM(bpm);

	for(t=0;t<numsmp;t++){
		samples[t].handle=driver->PrepareSample(samples[t].msample);
	}
	
	isready=0;
	isplaying=1;
}




void MMODULE_MOD::Stop()
{
	int t;

    if(!isplaying) return;

	// free all voices
	for(t=0;t<numchn;t++){
		driver->FreeVoice(audio[t].voice);
	}

	// disconnect samples
	for(t=0;t<numsmp;t++){
		driver->UnPrepareSample(samples[t].handle);
		samples[t].handle=-1;
	}

	isplaying=0;
}



void MMODULE_MOD::Update()
{
	if(!isplaying || isready ) return;

	if(++tick>=speed){

		row++;
		tick=0;

		if(patdly){
			patdly2=patdly;
			patdly=0;
		}

		if(patdly2){

			/* patterndelay active */

			if(--patdly2){
				row--;		/* so turn back mp_patpos by 1 */
			}
		}

		/* if this was the last row, or if a patternbreak is active ->
		   increase the song position */

		if(row>=64 || breakpos || posjump){
			row=0;

			if(posjump){
				if(!loopmode && (posjump-1) <= songpos){
					isready=1;
					return;
				}
				songpos=posjump-1;
				posjump=0;
			}
			else{
				songpos++;
			}

			if(songpos>=songlength){
                if(!loopmode){
					isready=1;
					return;
				}
				songpos=0;			// restart;
			}
			else if(songpos<0)
				songpos=songlength-1;

			if(breakpos){
				breakpos--;
				if(breakpos<64) row=breakpos;
				breakpos=0;
			}
		}
	}

	MODPATTERN *pattern=&patterns[positions[songpos]];
	MODNOTE *rowptr=&(pattern->data[row*numchn]);

	int t;

	for(t=0;t<numchn;t++){
		audio[t].HandleTick(rowptr->nte,rowptr->ins,rowptr->eff,rowptr->dat);
		rowptr++;
	}

	/* process the envelopes & start the samples */

	for(t=0;t<numchn;t++){

		AUDTMP *a=&audio[t];

		if(a->kick){

			MODSAMPLE *s=&samples[a->sample];

			if(s->handle>=0){
				
				ULONG start=a->start,
					  length=s->length,
					  loops=s->loopstart,
					  loope=s->loopend;

				if( (s->flags&SF_LOOP && (start<loope)) ||
					(start<length) ){

					driver->VoicePlay(a->voice,
									s->handle,
									start,
									length,
									loops,
									loope,
									0,0,
									s->flags|SF_FTPAN);
				}
			}
			a->kick=0;
		}

		driver->VoiceSetVolume(a->voice,(255U*a->volume)/64);
		driver->VoiceSetPanning(a->voice,a->panning);
		if(a->period) driver->VoiceSetFrequency(a->voice,3579546UL / a->period);
	}

	driver->ampfactor=(float)pow(driver->channels,0.52);
	driver->SetBPM(bpm);

	timestamp+=(125L*1000)/(50L*bpm);
}


int MMODULE_MOD::GetTimeStamp()
{
	return timestamp;
}


void MMODULE_MOD::SetTimeStamp(int ts)
{
	timestamp=ts;
}

