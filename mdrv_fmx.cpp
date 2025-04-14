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
				1.07 - v1.00? added dithering mode
				1.08 - v1.00? temporary 1-channel mixing buffer no longer static, but a member of MDRIVER_FMX
*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include "mdrv_fmx.h"

/*
Hmmm.. just to let you know that I *THINK* I've come up with a final
version of filters.

First - the theoretical shit :)

A resonant filter can be determined via 3 coefficients: a.x(k) + b.y(k-1) +
c.y(k-2) (I'm using functions of k as that is how they are taught in
discrete time control systems here :) )

Straight theory under ideal conditions (where the sampling rate is really
high) provides these solutions:

a = 1/(1+d+e)
b = (d+2e)/(1+d+e)
c = -e/(1+d+e)

where 
d = 2*(damping factor)*(sampling rate)/(natural frequency)
e = (sampling rate/natural frequency)^2

Hence, I've made 2 lookup tables - one for 2*damping factor and one for
1/(natural frequency) instead of 2 lookup tables for filter coefficients.
This allows the song to be playback frequency inspecific.

Just a few other clarifications:

natural frequency is a value in rad/s. It's equal to 2*PI*Frequency in Hz.
I've included both of the source files I used to create the two lookup
tables. Excuse the code - it looks awful :) Freq.C is the 1/(natural
frequency) table, and res.c is the 2*damping factor table.

After some thought (yes! There was some involved) and some testing, I've
modified the calculation of d to be:

d = 2*(damping factor)*(sampling rate)/(natural frequency) + 2*(damping
factor)-1.

This seems to overcome the problem I've been trying to describe to you,
where I couldn't get a significant gain at resonance for high frequencies.

Any idea whether you'll have time to implement all of this?  I was thinking
about a close-to-start-of-december release for 'patch #3' :)

Cya round, mate!

Jeffrey Lim
*/



MDRIVER_FMX::MDRIVER_FMX() : MDRIVER()
{
}



MDRIVER_FMX::~MDRIVER_FMX()
{
}



void MDRIVER_FMX::Init()
{
    peak=0.0;
    for(int t=0;t<MAXSMP;t++){
        Samples[t]=NULL;
    }
    ditherl=NULL;
    ditherr=NULL;
}



void MDRIVER_FMX::Exit()
{
}



void MDRIVER_FMX::Start()
{
    int t;

    if(channels>MAXCHN) channels=MAXCHN;

    for(t=0;t<channels;t++){
        channel[t].parent=this;
        channel[t].allocated=0;
        channel[t].kick=0;
        channel[t].active=0;
        channel[t].volume=0;
        channel[t].samplehandle=0;      
        channel[t].keyon=0;
        channel[t].fadecount=0;
        channel[t].filtertemp1=0.0;
        channel[t].filtertemp2=0.0;
        channel[t].filtercutoff=0x7f;
        channel[t].filterdamping=0;

        dying[t]=channel[t];

/*      channel[t].orgflags=flags;
        channel[t].orgreppos=reppos;
        channel[t].orgrepend=repend;
        channel[t].susreppos=sreppos;
        channel[t].susrepend=srepend;
    
        channel[t].flags=flags;
        channel[t].samplehandle=handle;
        channel[t].start=start;
        channel[t].size=size;
        channel[t].kick=1;
        channel[t].keyon=0;
*/  }
    
    samplesthatfit=TICKBUFFERSIZE;
    if(mode & DMODE_STEREO) samplesthatfit>>=1;

    // a noclick mode is also interpolated mode:
    if(mode & DMODE_NOCLICK) mode|=DMODE_INTERP;

    // do not dither 8 bits modes
//  if(!(mode & DMODE_16BITS)) mode &=~DMODE_DITHER;

    // init dithering tables

    if(mode & DMODE_DITHER){
        double r;

        ditherl=new SWORD[DITHERSIZE];
        ditherr=new SWORD[DITHERSIZE];
        
        srand((unsigned)time(NULL));

        if(ditherl!=NULL && ditherr!=NULL){
            for(t=0;t<DITHERSIZE;t++){
                r=(((float)rand()) / RAND_MAX) - 0.5;
                ditherl[t]=(SWORD)(32768.0*r);      
            }

            for(t=0;t<DITHERSIZE;t++){
                r=(((float)rand()) / RAND_MAX) - 0.5;
                ditherr[t]=(SWORD)(32768.0*r);      
            }

            ditherlidx=0;
            ditherridx=0;
        }
        else{
            delete ditherl;
            delete ditherr;
            mode&=~DMODE_DITHER;
        }
    }

    tickleft=0;
    mixposition=0;
}



void MDRIVER_FMX::Stop()
{
    delete[] ditherl;
    ditherl=NULL;
    delete[] ditherr; 
    ditherr=NULL;
}



void MDRIVER_FMX::SetBPM(UBYTE ibpm)
{
    bpm=ibpm;
}



UBYTE MDRIVER_FMX::GetBPM()
{
    return bpm;
}



int MDRIVER_FMX::AllocVoice()
{
    int handle;
    for(handle=0;handle<channels;handle++){
        if(!channel[handle].allocated){
            channel[handle].allocated=1;
            return handle;
        }
    }
    return -1;
}



void MDRIVER_FMX::FreeVoice(int handle)
{
    if(handle<0) return;
    channel[handle].allocated=0;
}


void MDRIVER_FMX::UnPrepareSample(int handle)
{
    if(handle>=0) Samples[handle]=NULL;
}


int MDRIVER_FMX::PrepareSample(MSAMPLE *s)
{
    int handle;

    if(s==NULL) return -1;

    /* Find empty slot to put sample address in */

    for(handle=0;handle<MAXSMP;handle++){
        if(Samples[handle]==NULL) break;
    }

    if(handle==MAXSMP || s->data==NULL){
        // myerr=ERROR_OUT_OF_HANDLES;
        return -1;
    }

    Samples[handle]=s->data;
    return handle;
}



int MDRIVER_FMX::samples2bytes(int v)
{
    if(mode & DMODE_16BITS) v<<=1;
    if(mode & DMODE_STEREO) v<<=1;
    return v;
}



int MDRIVER_FMX::bytes2samples(int v)
{
    if(mode & DMODE_16BITS) v>>=1;
    if(mode & DMODE_STEREO) v>>=1;
    return v;
}


/*
#define Get16BitSample                              \
    {                                               \
        int a=srce[idx>>FRACBITS];                  \
        int b=srce[1+(idx>>FRACBITS)];              \
        s=a+(((b-a)*(idx&FRACMASK))>>FRACBITS);     \
    }

#define Get8BitSample                               \
    {                                               \
        int a=srce[idx>>FRACBITS]<<8;               \
        int b=srce[1+(idx>>FRACBITS)]<<8;           \
        s=a+(((b-a)*(idx&FRACMASK))>>FRACBITS);     \
    }

#define TwoPoleFilter(in,out,c1,c2,c3,t1,t2)        out=(c1*in)+(c2*t1)+(c3*t2); t2=t1; t1=out;
*/

static void ResampleNormal08(const SBYTE *srce,real *dest,SLONG idx,SLONG inc,UWORD todo)
{
    while(todo--){
        *dest=real(srce[idx>>FRACBITS]<<8);
        idx+=inc;
        dest++;
    }
}


static void ResampleNormal16(const SWORD *srce,real *dest,SLONG idx,SLONG inc,UWORD todo)
{
    while(todo--){
        *dest=real(srce[idx>>FRACBITS]);
        idx+=inc;
        dest++;
    }
}

// #define interp8(a,b,idx) (a<<8)+(((b-a)*(idx&FRACMASK))>>(FRACBITS-8))

#define resample8(d)    a=srce[idx>>FRACBITS];                                  \
                        b=srce[1+(idx>>FRACBITS)];                              \
                        d=real((a<<8)+(((b-a)*(idx&FRACMASK))>>(FRACBITS-8)));  \
                        idx+=inc;

#define resample16(d)   a=srce[idx>>FRACBITS];                                  \
                        b=srce[1+(idx>>FRACBITS)];                              \
                        d=real(a+(((b-a)*(idx&FRACMASK))>>FRACBITS));           \
                        idx+=inc;

// #pragma optimize("a",on) 
    
static void ResampleInterp08(const SBYTE *srce,real *dest,SLONG idx,SLONG inc,UWORD todo)
{
    int a,b;
    
    while(todo>=8){
        resample8(dest[0])
        resample8(dest[1])
        resample8(dest[2])
        resample8(dest[3])
        resample8(dest[4])
        resample8(dest[5])
        resample8(dest[6])
        resample8(dest[7])
        dest+=8;
        todo-=8;
    }

    while(todo--){
        resample8(*dest);
        dest++;
    }
}


static void ResampleInterp16(const SWORD *srce,real *dest,SLONG idx,SLONG inc,UWORD todo)
{
    int a,b;
    
    while(todo>=8){
        resample16(dest[0])
        resample16(dest[1])
        resample16(dest[2])
        resample16(dest[3])
        resample16(dest[4])
        resample16(dest[5])
        resample16(dest[6])
        resample16(dest[7])
        dest+=8;
        todo-=8;
    }

    while(todo--){
        resample16(*dest);
        dest++;
    }
}



void MDRIVER_FMX::VOICEPARMS::Resample(void *srce,real *dest,double idx,double inc,UWORD todo)
{
    if(flags&SF_16BITS){
        SWORD *source=(SWORD *)srce;
        SLONG index,increment;
        
        source+=int(floor(idx));
        index=(SLONG)((idx-floor(idx))*(1<<FRACBITS));
        increment=(SLONG)(inc*(1<<FRACBITS));
        
        if(parent->mode & DMODE_INTERP)
            ResampleInterp16(source,dest,index,increment,todo);
        else
            ResampleNormal16(source,dest,index,increment,todo);
    }
    else{
        SBYTE *source=(SBYTE *)srce;
        SLONG index,increment;

        source+=int(floor(idx));
        index=(SLONG)((idx-floor(idx))*(1<<FRACBITS));
        increment=(SLONG)(inc*(1<<FRACBITS));
        
        if(parent->mode & DMODE_INTERP)
            ResampleInterp08(source,dest,index,increment,todo);
        else
            ResampleNormal08(source,dest,index,increment,todo);
    }
}



/*
void MDRIVER_FMX::VOICEPARMS::ResonantFilter(real *srce,real *dest,real c1,real c2,real c3,real &tmp1,real &tmp2,int todo)
{
    real in,out,t1,t2;
    
    t1=tmp1; t2=tmp2;

    while(todo--){
        in=*srce++;
        out=(c1*in)+(c2*t1)+(c3*t2); t2=t1; t1=out;
        *dest++=out;
    }
    
    tmp1=t1; tmp2=t2;
}
*/


void MDRIVER_FMX::VOICEPARMS::ResonantFilter(const real *srce,real *dest,real c1,real c2,real c3,real &tmp1,real &tmp2,int todo)
{
    real o0,o1,o2,o3;
    
    o1=tmp1; o0=tmp2;

    while(todo>3){
        dest[0]=o2=(c1*srce[0])+(c2*o1)+(c3*o0);
        dest[1]=o3=(c1*srce[1])+(c2*o2)+(c3*o1);
        dest[2]=o0=(c1*srce[2])+(c2*o3)+(c3*o2);
        dest[3]=o1=(c1*srce[3])+(c2*o0)+(c3*o3);
        srce+=4;
        dest+=4;
        todo-=4;
    }

    while(todo--){
        dest[0]=o2=(c1*srce[0])+(c2*o1)+(c3*o0);
        srce++;
        dest++;
        o0=o1;
        o1=o2;
    }

    tmp1=o1; tmp2=o0;
}


void MDRIVER_FMX::VOICEPARMS::RampMix(const real *srce,real *dest,real voll,real volr,real incl,real incr,int todo)
{
    if(parent->mode & DMODE_STEREO){
        while(todo--){
            dest[0]+=(*srce)*voll;  
            dest[1]+=(*srce)*volr;
            srce++;
            dest+=2;
            voll+=incl;
            volr+=incr;
        }
    }
    else{
        while(todo--){
            dest[0]+=(*srce)*voll;  
            srce++;
            dest++;
            voll+=incl;
        }
    }
}


void MDRIVER_FMX::VOICEPARMS::NoRampMix(const real *srce,real *dest,real voll,real volr,int todo)
{
    if(parent->mode & DMODE_STEREO){
        while(todo--){
            dest[0]+=(*srce)*voll;  
            dest[1]+=(*srce)*volr;
            srce++;
            dest+=2;
        }
    }
    else{
        while(todo--){
            dest[0]+=(*srce)*voll;  
            srce++;
            dest++;
        }
    }
}


void MDRIVER_FMX::VOICEPARMS::MixRamp(void *srce,real *dest,double idx,double inc,UWORD todo)
{
    Resample(srce,parent->tempbuffer,idx,inc,todo);
    
    if(filtercutoff!=0x7f || filterdamping!=0) ResonantFilter(parent->tempbuffer,parent->tempbuffer,filterc1,filterc2,filterc3,filtertemp1,filtertemp2,todo);
    
    RampMix(parent->tempbuffer,dest,folvolmul,forvolmul,flvolinc,frvolinc,todo);
    folvolmul+=(flvolinc*todo);
    forvolmul+=(frvolinc*todo);
}



void MDRIVER_FMX::VOICEPARMS::MixNoRamp(void *srce,real *dest,double idx,double inc,UWORD todo)
{
    if(volume){
        Resample(srce,parent->tempbuffer,idx,inc,todo);
        
        if(filtercutoff!=0x7f || filterdamping!=0) ResonantFilter(parent->tempbuffer,parent->tempbuffer,filterc1,filterc2,filterc3,filtertemp1,filtertemp2,todo);
        
        NoRampMix(parent->tempbuffer,dest,flvolmul,frvolmul,todo);
    }
}



void MDRIVER_FMX::VOICEPARMS::UpdateIndex()
{
    double idxsize,idxlpos,idxlend;

    idxsize=size;
    idxlpos=reppos;
    idxlend=repend;

    /* update the 'current' index so the sample loops, or
       stops playing if it reached the end of the sample */

    if(isreverse){

        /* The sample is playing in reverse */

        if(islooping){

            /* the sample is looping, so check if
               it reached the loopstart index */

            if(current<idxlpos){
            
                if(isbidi){

                    /* sample is doing bidirectional loops, so 'bounce'
                        the current index against the idxlpos */

                    current=idxlpos+(idxlpos-current);
                    isreverse=0;
                    increment=-increment;
                }
                else{
                    /* normal backwards looping, so set the
                    current position to loopend index */

                    current=idxlend-(idxlpos-current);
                }
            }
        }
        else{

            /* the sample is not looping, so check
                if it reached index 0 */

            if(current<=0.0){

                /* playing index reached 0, so stop
                playing this sample */

                current=0.0;
                active=0;
            }
        }
    }
    else{

        /* The sample is playing forward */

        if(islooping){

            /* the sample is looping, so check if
                it reached the loopend index */

            if(current>idxlend){
    
                if(isbidi){

                    /* sample is doing bidirectional loops, so 'bounce'
                    the current index against the idxlend */

                    isreverse=1;
                    increment=-increment;
                    current=idxlend-(current-idxlend); /* ?? */
                }
                else{
                    /* normal backwards looping, so set the
                        current position to loopend index */

                    current=idxlpos+(current-idxlend);
                }
            }
        }
        else{

            /* sample is not looping, so check
                if it reached the last position */

            if(current>=idxsize){

                /* yes, so stop playing this sample */

                current=0.0;
                active=0;
            }
        }
    }
}



void MDRIVER_FMX::VOICEPARMS::AddChannel(real *buffer,UWORD todo)
/*
    Mixes 'todo' stereo or mono samples of the current channel to the tickbuffer.
*/
{
    void  *s;
    real  *ptr;

    ptr=buffer;

    while(todo>0){

        /* update the 'current' index so the sample loops, or
           stops playing if it reached the end of the sample */

        UpdateIndex();

        if(!active) break;

        /* Vraag een far ptr op van het sampleadres
            op byte offset vnf->current, en hoeveel samples
            daarvan geldig zijn (VOORDAT segment overschrijding optreed) */

        if(!(s=parent->Samples[samplehandle])){
            current=0.0;
            active=0;
            break;
        }

        double end;
    
        if(isreverse)
            end = (islooping) ? reppos : 0.0;
        else
            end = (islooping) ? repend : size;

        /* Als de sample simpelweg niet beschikbaar is, of als
            sample gestopt moet worden sample stilleggen en stoppen */
        /* mix 'em: */

		SLONG done=(SLONG)((end-current)/increment); done++;
		if(done>todo) done=todo;

		if(!done){
			active=0;
			current=0.0;
			break;
		}

		if((parent->mode & DMODE_NOCLICK) && fadecount){

			/* limit the number of samples to mix to the fadecount: */

			if(done>fadecount) done=fadecount;

			MixRamp(s,ptr,current,increment,(UWORD)done);
/*
#ifndef DUMMYMIX
			if(vnf->flags&SF_16BITS){
				if(mode & DMODE_STEREO)	fMix16StereoNoclick((SWORD *)s,ptr,vnf->current,vnf->increment,done,vnf->folvolmul,vnf->forvolmul,vnf->flvolinc,vnf->frvolinc);
			}
			else{
				if(mode & DMODE_STEREO)	fMix08StereoNoclick((SBYTE *)s,ptr,vnf->current,vnf->increment,done,vnf->folvolmul,vnf->forvolmul,vnf->flvolinc,vnf->frvolinc);
			}
#endif
*/			// advance the volume pointer  !! NO this is done in the mixing routine itself

//			vnf->folvolmul+=(vnf->flvolinc*done);
//			vnf->forvolmul+=(vnf->frvolinc*done);

			fadecount-=(UWORD)done;

			if(!fadecount){
				folvolmul=flvolmul;
				forvolmul=frvolmul;
				flvolinc=0;
				frvolinc=0;
			}
		}
		else{
			/* normal clicky mode */
			MixNoRamp(s,ptr,current,increment,(UWORD)done);
		}

		// advance sample pointer

		current+=(increment*done);

		todo-=(UWORD)done;
		ptr+=(parent->mode & DMODE_STEREO) ? (done<<1) : done;
	}
}





void FloatTo8Copy(real *srce,SBYTE *dest,ULONG count)
{
	real  s;
	SLONG c;

	while(count--){
#ifdef MIKIT_WINDOWS
		s=*srce; // * 128.0;
		__asm{
			fld    s
			fistp  c
		}
#elif defined(MIKIT_WATCOM)
                _asm{
                        mov    eax,srce
                        fld    dword ptr [eax]
                        fistp  c
                }
#else
//              c=SLONG(*srce);
//		asm("flds   %1
//                     fistpl %0" : "=ms"(c) : "mf"(*srce));
//		asm("flds   %0" : : "mf" (s));
//		asm("fistpl %0" : "=ms" (c));
		c=SLONG(*srce);		
#endif
		if(c>127) c=127;
		else if(c<-128) c=-128;
		*dest++=(SBYTE)(c+128);
		srce++;
	}
}


/*
void FloatTo16Copy(real *srce,SWORD *dest,ULONG count)
{
	real  s;
	SLONG c;

	while(count--){
#ifdef MIKIT_WINDOWS
                s=*srce; //* 32768.0;
		__asm{
			fld    s
			fistp  c
		}
#elif defined(MIKIT_WATCOM)
                _asm{
                        mov    eax,srce
                        fld    dword ptr [eax]
                        fistp  c
                }
#else
//              c=SLONG(*srce);
		asm("flds   %1
                     fistpl %0" : "=ms"(c) : "mf"(*srce));
//		asm("flds   %0" : : "mf" (s));
//		asm("fistpl %0" : "=ms" (c));
#endif
		if(c>32767) c=32767;
		else if(c<-32768) c=-32768;
		*dest++=(SWORD)c;
		srce++;
	}
}
*/


void FloatTo16Copy(real *srce,SWORD *dest,ULONG count)
{
	real  s;
	SLONG c;

	while(count--){
#ifdef MIKIT_WINDOWS
        s=*srce;

		__asm{
			fld    s
			fistp  c
		}
#elif defined(MIKIT_WATCOM)
                _asm{
                        mov    eax,srce
                        fld    dword ptr [eax]
                        fistp  c
                }
#else
//              c=SLONG(*srce);
//		asm("flds   %1
//                     fistpl %0" : "=ms"(c) : "mf"(*srce));
//		asm("flds   %0" : : "mf" (s));
//		asm("fistpl %0" : "=ms" (c));
		c=SLONG(*srce);
#endif
		if(c>32767) c=32767;
		else if(c<-32768) c=-32768;
		*dest++=(SWORD)c;
		srce++;
	}
}


void MDRIVER_FMX::Dither(real *buf,SWORD *dith,int &index,int skip,int todo)
{
	while(todo){
		int part=((index+todo) > DITHERSIZE) ? (DITHERSIZE-index) : todo;
		todo-=part;

		while(part){
			(*buf)+=(real)(dith[index++]/32768.0);
			buf+=skip;
			part--;
		}

		if(index>=DITHERSIZE) index=0;
	}
}


void MDRIVER_FMX::WritePortion(SBYTE *buf,int todo)
/*
	Mixes 'todo' samples to 'buf'.. The number of samples has
	to fit into the tickbuffer.
*/
{
	int t;

	/* clear the mixing buffer: */

	if(mode&DMODE_STEREO){
		for(t=0;t<(todo<<1);t++) tfltbuffer[t]=0.0;
	}
	else{
		for(t=0;t<todo;t++) tfltbuffer[t]=0.0;
	} 

	/* add each channel one-by-one */

	if(mode & DMODE_NOCLICK){
		for(t=0;t<channels;t++){
			if(dying[t].active && dying[t].fadecount) dying[t].AddChannel(tfltbuffer,todo);
		}
	}

	for(t=0;t<channels;t++){
		if(channel[t].active) channel[t].AddChannel(tfltbuffer,todo);
	}

	/* dither the endmix ? */

	if(mode & DMODE_DITHER){
		if(mode & DMODE_STEREO){
			Dither(tfltbuffer  ,ditherl,ditherlidx,2,todo);
			Dither(tfltbuffer+1,ditherr,ditherridx,2,todo);
		}
		else{
			Dither(tfltbuffer  ,ditherl,ditherlidx,1,todo);
		}
	}

	if(mode & DMODE_SCAN){
		int nr=todo; if(mode & DMODE_STEREO) nr<<=1;
		for(t=0;t<nr;t++){
			if(fabs(tfltbuffer[t])>peak) peak=fabs(tfltbuffer[t]);
		}
	}
	else{
		if(mode & DMODE_16BITS)
			FloatTo16Copy(tfltbuffer,(SWORD *)buf,(mode & DMODE_STEREO) ? todo<<1 : todo);
		else
			FloatTo8Copy(tfltbuffer,buf,(mode & DMODE_STEREO) ? todo<<1 : todo);
	}
#ifdef MAELCUMHACK
	{
		int nr=todo; if(mode & DMODE_STEREO) nr<<=1;
		for(t=0;t<nr;t++){
			if(fabs(tfltbuffer[t])>peak) peak=fabs(tfltbuffer[t]);
		}
	}
#endif
}

/*
#define reson  0.8
#define TWO_PI 6.28318530718
#define freq   400.0
#define SRATE  44100.0
*/

void MDRIVER_FMX::VOICEPARMS::NewParms()
{
	int pan,vol,lvol,rvol;

	// kick channel ?

	if(kick){
		current=start;
		active=1;
		kick=0;
		folvolmul=0.0;
		forvolmul=0.0;
		filtertemp1=0.0;
		filtertemp2=0.0;
	}

	// frequency sanity check

	if(frequency<1000) active=0;

	// compute other parameters if channel is active
				
	if(active){
		increment=(double)frequency / (double)parent->frequency;

		if(isreverse) increment=-increment;

		vol=volume;
		pan=panning;

		if(parent->mode & DMODE_STEREO){

			if(flags & SF_SURROUND){
				// surround, so no panning allowed
				flvolmul=(parent->maxfvol*vol)/510.0f;
				frvolmul=-flvolmul;
			}
			else{
				if(flags & SF_FTPAN){

					// fasttracker style panning
					lvol=(vol*((pan<128) ? 128 : (255-pan))) / 128;
					rvol=(vol*((pan>128) ? 128 : pan)) / 128;
				}
				else{
					// old style panning
					lvol= ( vol * (255-pan) ) / 255;
					rvol= ( vol * pan ) / 255;
				}

				flvolmul=(parent->maxfvol*lvol)/255.0f;
				frvolmul=(parent->maxfvol*rvol)/255.0f;
			}
		}
		else{
			flvolmul=(parent->maxfvol*vol)/255.0f;
			frvolmul=flvolmul;
		}

		/* fade to a new volume in 32 steps */

		fadecount=(UWORD)(parent->frequency/344);
		flvolinc=(flvolmul-folvolmul)/fadecount;
		frvolinc=(frvolmul-forvolmul)/fadecount;

		if(filtercutoff==0x7f && filterdamping==0){
			filterc1=1.0;
			filterc2=0.0;
			filterc3=0.0;
		}
		else{
			real natfrq;
			real dmpfac;
			real mixfrq=(real)parent->frequency;

			natfrq = (real)(2.0 * 3.14159265358 * 110.0 * pow(2, 0.25) * pow(2,(real) (filtercutoff) / 24.0));
			dmpfac = (real)(pow(10, -((24.0 / 128.0)*filterdamping) / 20.0));
			
//			real d = (2.0*dmpfac)*(mixfrq/natfrq) + (2.0*dmpfac) - 1.0;

			real d = (real)((1.0-2.0*dmpfac)*natfrq / mixfrq);
			if(d>2.0) d = 2.0;
			d = (real)((2.0*dmpfac - d)*mixfrq / natfrq);
			
			real e = pow((real)(mixfrq/natfrq),2.0f);
			
			filterc1=1/(1+d+e);
			filterc2=(d+e+e)/(1+d+e);
			filterc3=-e/(1+d+e);
		}
	}
}



void MDRIVER_FMX::WriteSamples(SBYTE *buffer,int todo)
{
	int t,part;

	while(todo>0){

		if(tickleft==0){

			/* preserve channels */

			if(mode & DMODE_NOCLICK) for(t=0;t<channels;t++) dying[t]=channel[t];

			/* call tickhandler routine, if present */
			
			Tick();
		
			/* new 13-March-1998 : dynamic volume calculation depending on uservolume */
			
			maxfvol=ampfactor;		// pow(channels,0.52);
			maxfvol/=channels;
			//	maxfvol/=32768.0;
			if(!(mode & DMODE_16BITS)) maxfvol/=256.0f;
			maxfvol=(maxfvol*uservolume)/100.0f;

			// determine new number of ticks left before tickhandler should be called again 
			
			tickleft=(125L*frequency)/(50L*bpm);

			/* compute volume, frequency counter & panning parameters for each channel. */

			for(t=0;t<channels;t++){

				if(mode & DMODE_NOCLICK){
					if(dying[t].active && (channel[t].kick || channel[t].active==0)){
						dying[t].volume=0;
						dying[t].NewParms();
					}
					else{
						dying[t].active=0;
					}
				}
				
				channel[t].NewParms();
			}
		}

		part=min(tickleft,todo);
		part=min(part,samplesthatfit);

		WritePortion(buffer,part);

		tickleft-=part;
		todo-=part;
		mixposition+=part;

		buffer+=samples2bytes(part);
	}
}


ULONG MDRIVER_FMX::GetMixPosition()
{
	return mixposition;
}


int MDRIVER_FMX::WriteBytes(SBYTE *buffer,int todo)
{
	todo=bytes2samples(todo);
	WriteSamples(buffer,todo);
	return samples2bytes(todo);
}


void MDRIVER_FMX::WriteSilentSamples(SBYTE *buffer,int todo)
{
}


int MDRIVER_FMX::WriteSilentBytes(SBYTE *buffer,int todo)
{
	if(mode&DMODE_16BITS){
		memset(buffer,0,todo);
	}
	else{
		memset(buffer,0x80,todo);
	}
	return todo;
}


void MDRIVER_FMX::VoiceSetVolume(int voice,UBYTE volume)
{	
	if(voice<0) return;
	channel[voice].volume=volume;
}



void MDRIVER_FMX::VoiceSetPanning(int voice,UBYTE panning)
{	
	if(voice<0) return;
	channel[voice].panning=panning;
}



void MDRIVER_FMX::VoiceSetFrequency(int voice,ULONG frequency)
{	
	if(voice<0) return;
	channel[voice].frequency=frequency;
}



void MDRIVER_FMX::VoiceSetFilter(int voice,UBYTE cutoff,UBYTE damping)
{	
	if(voice<0) return;
	channel[voice].filtercutoff=(cutoff<127) ? cutoff : 127;
	channel[voice].filterdamping=(damping<127) ? damping : 127 ;
}



void MDRIVER_FMX::VoiceStop(int voice)
{	
	if(voice<0) return;
	channel[voice].kick=0;
	channel[voice].active=0;
}



int MDRIVER_FMX::VoiceActive(int voice)
{	
	if(voice<0) return 0;
	return channel[voice].active;
}



void MDRIVER_FMX::VoiceKeyOff(int voice)
{
	if(channel[voice].keyon){

		// 1.04 - v0.92 fixed bug : on a noteoff, when going from a sustain loop to a normal loop, make a sample go forward

		if(channel[voice].orgflags & SF_SUSLOOP){		
			channel[voice].isreverse=0;						
		}

		if(channel[voice].orgflags & SF_LOOP){
			channel[voice].islooping=1;
			channel[voice].isbidi=(channel[voice].orgflags & SF_BIDI) ? 1 : 0;
			channel[voice].reppos=channel[voice].orgreppos;
			channel[voice].repend=channel[voice].orgrepend;
		}
		else{
			channel[voice].islooping=0;
			channel[voice].isbidi=0;
			channel[voice].isreverse=0;
		}
		channel[voice].keyon=0;
	}
}



void MDRIVER_FMX::VoiceKeyOn(int voice)
{
	if(!channel[voice].keyon){
		if(channel[voice].orgflags & SF_SUSLOOP){
			channel[voice].isreverse=0;
			channel[voice].islooping=1;
			channel[voice].isbidi=(channel[voice].orgflags & SF_SUSBIDI) ? 1 : 0;
			channel[voice].reppos=channel[voice].susreppos;
			channel[voice].repend=channel[voice].susrepend;
		}
		else if(channel[voice].orgflags & SF_LOOP){
			channel[voice].isreverse=0;
			channel[voice].islooping=1;
			channel[voice].isbidi=(channel[voice].orgflags & SF_BIDI) ? 1 : 0;
			channel[voice].reppos=channel[voice].orgreppos;
			channel[voice].repend=channel[voice].orgrepend;
		}
		else{
			channel[voice].isreverse=0;
			channel[voice].islooping=0;
			channel[voice].isbidi=0;
		}
		channel[voice].keyon=1;
	}
}



void MDRIVER_FMX::VoicePlay(int voice,int handle,ULONG start,ULONG size,ULONG reppos,ULONG repend,ULONG sreppos,ULONG srepend,UWORD flags)
{
	if(voice<0) return;
	if(handle<0) return;
	if(start>=size) return;

	if(flags&SF_LOOP){
		if(repend>size) repend=size;    /* repend can't be bigger than size */
    }

    channel[voice].orgflags=flags;
    channel[voice].orgreppos=reppos;
    channel[voice].orgrepend=repend;
    channel[voice].susreppos=sreppos;
    channel[voice].susrepend=srepend;

    channel[voice].flags=flags;
    channel[voice].samplehandle=handle;
    channel[voice].start=start;
    channel[voice].size=size;
    channel[voice].kick=1;
    channel[voice].keyon=0;

    VoiceKeyOn(voice);
}


void MDRIVER_FMX::Update()
{
    WriteBytes(NULL,4096);  
}
