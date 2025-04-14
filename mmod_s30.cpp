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

File:           MMOD_S30.CPP
Description:    -
Version:        1.00 - original
                1.01 - create one extra fake pattern to use for illegal pattern numbers

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mmod_s3m.h"


int MMODULE_S3M::Test(MINPUT *i,char *title,int maxlen)
{
    UBYTE id[4];

    i->setmode(MINPUT::INTEL);
    i->rewind();
    i->seek(0x2c,SEEK_SET);
    if(!i->read_UBYTES(id,4)) return 0;
    if(!memcmp(id,"SCRM",4)) return 1;
    return 0;
}


int MMODULE_S3M::GetPos()
{
    if(!isplaying) return 0;
    return songpos;
}


void MMODULE_S3M::NextPos()
{
    if(!isplaying) return;
    for(int t=0;t<numchn;t++) driver->VoiceStop(audio[t].voice);
    posjump=songpos+2;
}


void MMODULE_S3M::PrevPos()
{
    if(!isplaying) return;
    if(songpos){
        for(int t=0;t<numchn;t++) driver->VoiceStop(audio[t].voice);
        posjump=(UBYTE)songpos;
    }
}


int MMODULE_S3M::GetNumChannels()
{
    return numchn;
}


int MMODULE_S3M::GetNumSamples()
{
    return numsmp;
}

char *MMODULE_S3M::GetSampleName(int s)
{
    return samples[s].name;
}


char *MMODULE_S3M::GetSongTitle()
{
    return songname;
}


int MMODULE_S3M::GetSongLength()
{
    return songlength;
}


int MMODULE_S3M::IsReady()
{
    return isready;
}


MMODULE_S3M::MMODULE_S3M() : MMODULE()
{
    /* hier members initialiseren */

    patterns=NULL;
    samples=NULL;
    isplaying=0;

    smppara=NULL;
    patpara=NULL;
}


void MMODULE_S3M::Load(MINPUT *i)
{
    int t;

    in=i;

    moduletype="Screamtracker III";
    
    /* try to read module header */

    i->setmode(MINPUT::INTEL);
    i->rewind();
    i->read_STRING(songname,28);    // song name
    i->read_UBYTE();                // 0x1a
    s3mtype=i->read_UBYTE();        // screamtracker type
    i->seek(0x2,SEEK_CUR);          // unused
    songlength  =i->read_UWORD();   // song length
    numsmp      =i->read_UWORD();   // number of samples
    numpat      =i->read_UWORD();   // number of patterns
    s3mflags    =i->read_UWORD();   // s3m flags
    tracker     =i->read_UWORD();   // tracker version
    UWORD format=i->read_UWORD();   // format ?
    i->seek(0x4,SEEK_CUR);          // skip SCRM id
    mastervol   =i->read_UBYTE();   // master volume
    initspeed   =i->read_UBYTE();   // initial speed
    inittempo   =i->read_UBYTE();   // initial tempo
    mastermult  =i->read_UBYTE();   // master volume multiplier
    i->read_UBYTE();                // ultraclick ?
    UBYTE pantable=i->read_UBYTE(); // pantable flag
    i->seek(0x8,SEEK_CUR);          // 8 unused bytes
    i->seek(0x2,SEEK_CUR);          // skip 'special' para pointer
    i->read_UBYTES(channels,32);    // 32 channel flags

    if(infomode) return;            // don't load other stuff in infomode
    
	if(i->eof()) THROW MikModException("Header too short");

/*		printf("songname: %s\n",songname);
	printf("samples : %d\n",numsmp);
	printf("patterns: %d\n",numpat);
	printf("ispeed  : %d\n",initspeed);
	printf("itempo  : %d\n",inittempo);
*/				
	/* read positions and read them */

	if(songlength>256) THROW MikModException("Too many song positions");
	i->read_UBYTES(positions,songlength);

//		for(t=0;t<songlength;t++) printf("%d ",positions[t]);

	/* allocate and read sample para pointers */
	smppara=new UWORD[numsmp];
	i->read_UWORDS(smppara,numsmp);
	
	/* allocate and read pattern para pointers */
	patpara=new UWORD[numpat];
	i->read_UWORDS(patpara,numpat);

	/* read the panning table */

	memset(panning,0,32);

	if(pantable==252){
		i->read_UBYTES(panning,32);			
	}

	for(t=0;t<32;t++){
		if(panning[t]&0x10){
			panning[t]=panning[t]<<4;
		}
		else{
			if(channels[t]>15){		
				panning[t]=0x80;
			}
			else if(channels[t]>7){
				panning[t]=0xc0;
			}
			else{
				panning[t]=0x30;
			}
		}
	}

//		for(t=0;t<32;t++) printf("%02x ",channels[t]);
//		puts("");
//		for(t=0;t<32;t++) printf("%02x ",panning[t]);
//		puts("");

	numchn=0;
	memset(remap,255,32);
	
	for(t=0;t<32;t++){								// visit all channels
		if(channels[t]<16 && remap[t]==255){		// channel is active and hasn't been remapped already
            for(int j=t;j<32;j++){                  // visit this and all higher channels
                if(channels[j]==channels[t]){       // see if they point to the same logical channel
                    remap[j]=numchn;                // if yes, map them to the same hw channel
                }
            }
            numchn++;
        }
    }

//      for(t=0;t<32;t++) printf("%02x ",remap[t]);
//      puts("");

    /* allocate sample array */

    samples=new S3MSAMPLE[numsmp];

    /* load all samples */

    for(t=0;t<numsmp;t++){
        samples[t].flags=(format==1) ? SF_SIGNED : 0;       // init sample flags
        samples[t].Load(in,ULONG(smppara[t])<<4);   // and load sample
        TerminateThrow();
    }

    /* destroy para pointers */

    delete[] smppara;
    smppara=NULL;

    /* allocate pattern array */

    patterns=new S3MPATTERN[numpat+1];

    /* load all patterns */

    for(t=0;t<numpat;t++){
        patterns[t].Load(in,ULONG(patpara[t])<<4);
        TerminateThrow();
    }

    patterns[numpat].Fake();

    /* destroy para pointers */

    delete[] patpara;
    patpara=NULL;
}



MMODULE_S3M::~MMODULE_S3M()
{
    Stop();
    delete[] patterns;
    patterns=NULL;
    delete[] samples;
    samples=NULL;
    delete[] patpara;
    patpara=NULL;
    delete[] smppara;
    smppara=NULL;
}



void MMODULE_S3M::Restart()
{
    int t;

    isplaying=0;
    timestamp=0;
    
    speed=initspeed;
    tick=speed;
    bpm=inittempo;

    breakpos=0;
    posjump=0;
    patdly=0;
    patdly2=0;
    repcnt=0;
    reppos=0;

    songpos=0;
    row=-1;

    globalvolume=64;

    for(t=0;t<numchn;t++){
        int v=audio[t].voice;
        memset(&audio[t],0,sizeof(AUDTMP));
        audio[t].voice=v;
        audio[t].panning=0x80;
        audio[t].parent=this;
        audio[t].c2spd=8363;
    }

    for(t=0;t<32;t++){                              // visit all channels
        if(remap[t]<numchn){
            audio[remap[t]].panning=panning[t];     // set panning
        }
    }
    
    driver->SetBPM(bpm);
    isready=0;
    isplaying=1;
}



void MMODULE_S3M::Start(MDRIVER *d)
{
    int t;
    
    if(isplaying) return;

    driver=d;

    timestamp=0;
    
    speed=initspeed;
    tick=speed;
    bpm=inittempo;

    breakpos=0;
    posjump=0;
    patdly=0;
    patdly2=0;
    repcnt=0;
    reppos=0;

    songpos=0;
    row=-1;

    globalvolume=64;

    for(t=0;t<numchn;t++){
        memset(&audio[t],0,sizeof(AUDTMP));
        audio[t].voice=driver->AllocVoice();
        audio[t].panning=0x80;
        audio[t].parent=this;
        audio[t].c2spd=8363;
    }

    for(t=0;t<32;t++){                              // visit all channels
        if(remap[t]<numchn){
            audio[remap[t]].panning=panning[t];     // set panning
        }
    }
    
    driver->ampfactor=(float)pow(driver->channels,0.52);
    driver->SetBPM(bpm);

    // connect samples
    for(t=0;t<numsmp;t++){
        samples[t].handle=driver->PrepareSample(samples[t].msample);
    }

    isready=0;
    isplaying=1;
}



void MMODULE_S3M::Stop()
{
    int t;

    if(!isplaying) return;

    isplaying=0;

    // free all voices
    for(t=0;t<numchn;t++){
        driver->FreeVoice(audio[t].voice);
    }

    // disconnect samples
    for(t=0;t<numsmp;t++){
        driver->UnPrepareSample(samples[t].handle);
        samples[t].handle=-1;
    }
}



void MMODULE_S3M::Update()
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
                row--;      /* so turn back mp_patpos by 1 */
            }
        }

        /* if this was the last row, or if a patternbreak is active ->
           increase the song position */

        if(row>=64 || breakpos || posjump ){
            row=0;

            if(posjump){

                if(!loopmode && (posjump-1)<=songpos){
                    isready=1;
                    return;
                }

                songpos=posjump-1;
                posjump=0;
            }
            else{
                songpos++;
            }


            while(positions[songpos]==0xfe && songpos<songlength) songpos++;
            
            if(songpos>=songlength || positions[songpos]==0xff){
                if(!loopmode){
                    isready=1;
                    return;
                }
                songpos=0;          // restart;
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

    UBYTE patno=(UBYTE)(positions[songpos]); 
    if(patno>numpat) patno=(UBYTE)numpat;
    S3MPATTERN *pattern=&patterns[patno];

    S3MNOTE notes[32];

    // decode a whole s3m row

    pattern->DecodeRow(row,notes);

    int t;

    // remap the notes so they are neatly packed together

    for(t=0;t<32;t++){
        if(channels[t]<16) notes[remap[t]]=notes[t];
    }

/*  if(!tick){
        printf("%d - ",row); // ,songpos,positions[songpos]);
        for(t=0;t<numchn;t++) printf("%02x ",notes[t].ins);
        printf("\n");
    }
*/
    for(t=0;t<numchn;t++){
        audio[t].HandleTick(notes[t].nte,notes[t].ins,notes[t].vol,notes[t].eff,notes[t].dat);
    }

/*  if(!tick){
        printf("%d - ",row); // ,songpos,positions[songpos]);
        for(t=0;t<numchn;t++) printf("%x ",audio[t].note);
        printf("\n");
    }
*/

#ifdef NEVER
    int t;

    for(t=0;t<numchn;t++){
        audio[t].HandleTick(rowptr->nte,rowptr->ins,rowptr->eff,rowptr->dat);
        rowptr++;
    }
#endif

    /* process the envelopes & start the samples */

    if(!scanmode) for(t=0;t<numchn;t++){

        AUDTMP *a=&audio[t];

        if(a->kick && a->sample>0){

            S3MSAMPLE *s=&samples[a->sample-1];

            if(s->handle>=0){

                ULONG start=(a->ownofs) ? a->startoffset : 0,
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

        if(a->cut){
            driver->VoiceStop(a->voice);
            a->cut=0;
        }

        ULONG tvol=a->volume;
        tvol*=globalvolume;
        tvol*=255;
        tvol/=4096;
                
        driver->VoiceSetVolume(a->voice,(UBYTE)tvol);
        driver->VoiceSetPanning(a->voice,a->panning);
        if(a->period) driver->VoiceSetFrequency(a->voice,14317056UL / a->period);
    }

    driver->SetBPM(bpm);
    timestamp+=(125L*1000)/(50L*bpm);
}


int MMODULE_S3M::GetTimeStamp()
{
    return timestamp;
}


void MMODULE_S3M::SetTimeStamp(int ts)
{
    timestamp=ts;
}

