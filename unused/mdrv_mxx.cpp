/*

File:           MDRV_MXX.CPP
Description:    -
Version:        1.00 - original
                1.01 - allow fasttracker style panning using SF_FTPAN flag
                1.02 - v0.91 fixed bug : increase low frequency sanity check to prevent zero division (keith 303 module)
                1.03 - v0.91 fixed bug : allocate twice as much memory when loading a 16 bit sample PLEASE
                1.04 - v0.92 fixed bug : on a noteoff, when going from a sustain loop to a normal loop, make a sample go forward
*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <assert.h>
#include "mdrv_mxx.h"




MDRIVER_MXX::MDRIVER_MXX() : MDRIVER()
{
    for(int t=1;t<=MAXCHN;t++){
        amplifytable[t-1]=SLONG(65536.0*pow(t,0.52));
//      printf("%d\n",amplifytable[t-1]);   
    }
}



MDRIVER_MXX::~MDRIVER_MXX()
{
}



void MDRIVER_MXX::Init()
{
    for(int t=0;t<MAXSMP;t++){
        Samples[t]=NULL;
    }
}



void MDRIVER_MXX::Exit()
{
}



void MDRIVER_MXX::Start()
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
    
    maxvol=amplifytable[channels-1];
    maxvol*=uservolume;
    maxvol/=100;
    
    ampshift=0;

    /* maxvol can't be bigger than 65536, so rescale it using ampshift
	   until it's below or equal to 65536 */

    while(maxvol>65536L){
        ampshift++;
        maxvol>>=1;
    }

    maxvol=maxvol / channels;

    samplesthatfit=TICKBUFFERSIZE;
    if(mode & DMODE_STEREO) samplesthatfit>>=1;

    // a noclick mode is also interpolated mode:

    if(mode & DMODE_NOCLICK) mode|=DMODE_INTERP;

    tickleft=0;
}



void MDRIVER_MXX::Stop()
{
}



void MDRIVER_MXX::SetBPM(UBYTE ibpm)
{
    bpm=ibpm;
}



UBYTE MDRIVER_MXX::GetBPM()
{
    return bpm;
}



int MDRIVER_MXX::AllocVoice()
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



void MDRIVER_MXX::FreeVoice(int handle)
{
    if(handle<0) return;
    channel[handle].allocated=0;
}



int MDRIVER_MXX::SampleLoad(MINPUT *i,ULONG length,ULONG reppos,ULONG repend,UWORD flags)
{
    int handle;
    ULONG t;

//  printf("Loading a %ld sample sample\n",length);
    
    converter.Init(i,flags,flags|SF_SIGNED);

    /* Find empty slot to put sample address in */

    for(handle=0;handle<MAXSMP;handle++){
        if(Samples[handle]==NULL) break;
    }

    if(handle==MAXSMP){
        // myerr=ERROR_OUT_OF_HANDLES;
        return -1;
    }

    if(flags&SF_16BITS){
        Samples[handle]=malloc((length+16)<<1);
    }
    else{
        Samples[handle]=malloc(length+16);
    }

    if(Samples[handle]==NULL){
        // myerr=ERROR_SAMPLE_TOO_BIG;
        return -1;
    }

    /* read sample into buffer. */
    converter.ReadSamples(Samples[handle],length);

    /* Unclick samples: */

    if(flags & SF_16BITS){
        SWORD *s=(SWORD *)Samples[handle];

            if(flags & SF_LOOP){
                if(flags & SF_BIDI)
                    for(t=0;t<16;t++) s[repend+t]=s[(repend-t)-1];
                else
                    for(t=0;t<16;t++) s[repend+t]=s[t+reppos];
            }
            else{
                for(t=0;t<16;t++) s[t+length]=0;
            }
    }
    else{
        SBYTE *s=(SBYTE *)Samples[handle];

        if(flags & SF_LOOP){
            if(flags & SF_BIDI)
                for(t=0;t<16;t++) s[repend+t]=s[(repend-t)-1];
            else
                for(t=0;t<16;t++) s[repend+t]=s[t+reppos];
        }
        else{
            for(t=0;t<16;t++) s[t+length]=0;
        }
    }

    return handle;
}



void MDRIVER_MXX::SampleFree(int handle)
{
    if(handle>=0){
                void *sampleadr=Samples[handle];
                free(sampleadr);
        Samples[handle]=NULL;
    }
}



SLONG MDRIVER_MXX::VOICEPARMS::fraction2long(ULONG dividend,UWORD divisor)
/*
    Converts the fraction 'dividend/divisor' into a fixed point longword.
*/
{
    ULONG whole,part;

    whole=dividend/divisor;
    part=((dividend%divisor)<<FRACBITS)/divisor;

    return((whole<<FRACBITS)|part);
}



int MDRIVER_MXX::samples2bytes(int v)
{
    if(mode & DMODE_16BITS) v<<=1;
    if(mode & DMODE_STEREO) v<<=1;
    return v;
}



int MDRIVER_MXX::bytes2samples(int v)
{
    if(mode & DMODE_16BITS) v>>=1;
    if(mode & DMODE_STEREO) v>>=1;
    return v;
}




void MDRIVER_MXX::Mix16StereoNoclick(SWORD *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG lvolmul,SLONG rvolmul,SLONG lvolinc,SLONG rvolinc)
{
    int a,b,s;

    while(todo>0){
        a=srce[index>>FRACBITS];
        b=srce[1+(index>>FRACBITS)];
        s=a+(((b-a)*(index&FRACMASK))>>FRACBITS);

        *(dest++)+=lvolmul*s;
        *(dest++)+=rvolmul*s;
        lvolmul+=lvolinc;
        rvolmul+=rvolinc;
        
        index+=increment;
        todo--;
    }
}




void MDRIVER_MXX::Mix08StereoNoclick(SBYTE *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG lvolmul,SLONG rvolmul,SLONG lvolinc,SLONG rvolinc)
{
    int a,b,s;

    while(todo>0){
        a=srce[index>>FRACBITS]<<8;
        b=srce[1+(index>>FRACBITS)]<<8;
        s=a+(((b-a)*(index&FRACMASK))>>FRACBITS);

        *(dest++)+=lvolmul*s;
        *(dest++)+=rvolmul*s;
        lvolmul+=lvolinc;
        rvolmul+=rvolinc;

        index+=increment;
        todo--;
    }
}



void MDRIVER_MXX::Mix16MonoNoclick(SWORD *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG volmul,SLONG volinc)
{
    int a,b,s;

    while(todo>0){
        a=srce[index>>FRACBITS];
        b=srce[1+(index>>FRACBITS)];
        s=a+(((b-a)*(index&FRACMASK))>>FRACBITS);

        *(dest++)+=volmul*s;
        volmul+=volinc;

        index+=increment;
        todo--;
    }
}




void MDRIVER_MXX::Mix08MonoNoclick(SBYTE *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG volmul,SLONG volinc)
{
    int a,b,s;

    while(todo>0){
        a=srce[index>>FRACBITS]<<8;
        b=srce[1+(index>>FRACBITS)]<<8;
        s=a+(((b-a)*(index&FRACMASK))>>FRACBITS);

        *(dest++)+=volmul*s;
        volmul+=volinc;

        index+=increment;
        todo--;
    }
}



void MDRIVER_MXX::Mix16StereoNormal(SWORD *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG lvolmul,SLONG rvolmul)
{
    SWORD sample;

    while(todo>7){
        sample=srce[index>>FRACBITS];   dest[ 0]+=lvolmul*sample;   dest[ 1]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[ 2]+=lvolmul*sample;   dest[ 3]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[ 4]+=lvolmul*sample;   dest[ 5]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[ 6]+=lvolmul*sample;   dest[ 7]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[ 8]+=lvolmul*sample;   dest[ 9]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[10]+=lvolmul*sample;   dest[11]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[12]+=lvolmul*sample;   dest[13]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[14]+=lvolmul*sample;   dest[15]+=rvolmul*sample;   index+=increment;

        dest+=16;
        todo-=8;
    }

    while(todo>0){
        sample=srce[index>>FRACBITS];
        *(dest++)+=lvolmul*sample;
        *(dest++)+=rvolmul*sample;
        index+=increment;
        todo--;
    }
}


void MDRIVER_MXX::Mix16MonoNormal(SWORD *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG volmul)
{
    SWORD sample;

    while(todo>7){
        sample=srce[index>>FRACBITS];   dest[0]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[1]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[2]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[3]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[4]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[5]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[6]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[7]+=volmul*sample;     index+=increment;

        dest+=8;
        todo-=8;
    }

    while(todo>0){
        sample=srce[index>>FRACBITS];
        *(dest++)+=volmul*sample;
        index+=increment;
        todo--;
    }
}



void MDRIVER_MXX::Mix16StereoInterp(SWORD *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG lvolmul,SLONG rvolmul)
{
    int a,b,s;

    while(todo>0){
        a=srce[index>>FRACBITS];
        b=srce[1+(index>>FRACBITS)];
        s=a+(((b-a)*(index&FRACMASK))>>FRACBITS);

        *(dest++)+=lvolmul*s;
        *(dest++)+=rvolmul*s;
        
        index+=increment;
        todo--;
    }
}


void MDRIVER_MXX::Mix16MonoInterp(SWORD *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG volmul)
{
    int a,b,s;

    while(todo>0){
        a=srce[index>>FRACBITS];
        b=srce[1+(index>>FRACBITS)];
        s=a+(((b-a)*(index&FRACMASK))>>FRACBITS);

        *(dest++)+=volmul*s;

        index+=increment;
        todo--;
    }
}



void MDRIVER_MXX::Mix08StereoNormal(SBYTE *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG lvolmul,SLONG rvolmul)
{
    SBYTE sample;

    lvolmul<<=8;
    rvolmul<<=8;

    while(todo>7){
        sample=srce[index>>FRACBITS];   dest[ 0]+=lvolmul*sample;   dest[ 1]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[ 2]+=lvolmul*sample;   dest[ 3]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[ 4]+=lvolmul*sample;   dest[ 5]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[ 6]+=lvolmul*sample;   dest[ 7]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[ 8]+=lvolmul*sample;   dest[ 9]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[10]+=lvolmul*sample;   dest[11]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[12]+=lvolmul*sample;   dest[13]+=rvolmul*sample;   index+=increment;
        sample=srce[index>>FRACBITS];   dest[14]+=lvolmul*sample;   dest[15]+=rvolmul*sample;   index+=increment;

        dest+=16;
        todo-=8;
    }

    while(todo>0){
        sample=srce[index>>FRACBITS];
        *(dest++)+=lvolmul*sample;
        *(dest++)+=rvolmul*sample;
        index+=increment;
        todo--;
    }
}


void MDRIVER_MXX::Mix08MonoNormal(SBYTE *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG volmul)
{
    SBYTE sample;

    volmul<<=8;

    while(todo>7){
        sample=srce[index>>FRACBITS];   dest[0]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[1]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[2]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[3]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[4]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[5]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[6]+=volmul*sample;     index+=increment;
        sample=srce[index>>FRACBITS];   dest[7]+=volmul*sample;     index+=increment;

        dest+=8;
        todo-=8;
    }

    while(todo>0){
        sample=srce[index>>FRACBITS];
        *(dest++)+=volmul*sample;
        index+=increment;
        todo--;
    }
}


void MDRIVER_MXX::Mix08StereoInterp(SBYTE *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG lvolmul,SLONG rvolmul)
{
    int a,b,s;

    while(todo>0){
        a=srce[index>>FRACBITS]<<8;
        b=srce[1+(index>>FRACBITS)]<<8;
        s=a+(((b-a)*(index&FRACMASK))>>FRACBITS);

        *(dest++)+=lvolmul*s;
        *(dest++)+=rvolmul*s;

        index+=increment;
        todo--;
    }
}


void MDRIVER_MXX::Mix08MonoInterp(SBYTE *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG volmul)
{
    int a,b,s;
    
    while(todo>0){
        a=srce[index>>FRACBITS]<<8;
        b=srce[1+(index>>FRACBITS)]<<8;
        s=a+(((b-a)*(index&FRACMASK))>>FRACBITS);

        *(dest++)+=volmul*s;

        index+=increment;
        todo--;
    }
}




UWORD MDRIVER_MXX::NewPredict(SLONG index,SLONG end,SLONG increment,UWORD todo)
/*
    This functions returns the number of resamplings we can do so that:

        - it never accesses indexes bigger than index 'end'
        - it doesn't do more than 'todo' resamplings
*/
{
	SLONG di;

	di=(end-index)/increment;
	index+=(di*increment);

	if(increment<0){
		while(index>=end){
			index+=increment;
			di++;
		}
	}
	else{
		while(index<=end){
			index+=increment;
			di++;
		}
	}
	return (UWORD)((di<todo) ? di : todo);
}



void MDRIVER_MXX::Sample32To8Copy(SLONG *srce,SBYTE *dest,ULONG count,UBYTE shift)
{
	SLONG c;

	while(count--){
		c=*srce >> shift;
#ifdef PEAKDETECTION
		if(abs(c)>peak) peak=abs(c);
#endif
		if(c>127) c=127;
		else if(c<-128) c=-128;
		*dest++=c+128;
		srce++;
	}
}



void MDRIVER_MXX::Sample32To16Copy(SLONG *srce,SWORD *dest,ULONG count,UBYTE shift)
{
	SLONG c;

	while(count--){
		c=*srce >> shift;
#ifdef PEAKDETECTION
		if(abs(c)>peak) peak=abs(c);
#endif
		if(c>32767) c=32767;
		else if(c<-32768) c=-32768;
		*dest++=(SWORD)c;
		srce++;
	}
}



void MDRIVER_MXX::AddChannel(VOICEPARMS *vnf,UWORD todo)
/*
	Mixes 'todo' stereo or mono samples of the current channel to the tickbuffer.
*/
{
    void  *s;
	SLONG *ptr,idxsize,idxlpos,idxlend;

	ptr=tickbuffer;

	idxsize=(vnf->size<<FRACBITS)-1;
	idxlpos=vnf->reppos<<FRACBITS;
	idxlend=(vnf->repend<<FRACBITS)-1;

	while(todo>0){

		/* update the 'current' index so the sample loops, or
		   stops playing if it reached the end of the sample */

		if(vnf->isreverse){

			/* The sample is playing in reverse */

			if(vnf->islooping){

				/* the sample is looping, so check if
				   it reached the loopstart index */

				if(vnf->current<idxlpos){
				
					if(vnf->isbidi){

						/* sample is doing bidirectional loops, so 'bounce'
							the current index against the idxlpos */
	
						vnf->current=idxlpos+(idxlpos-vnf->current);
						vnf->isreverse=0;
						vnf->increment=-vnf->increment;
					}
					else{
						/* normal backwards looping, so set the
						current position to loopend index */
	
						vnf->current=idxlend-(idxlpos-vnf->current);
					}
				}
			}
			else{

				/* the sample is not looping, so check
					if it reached index 0 */

				if(vnf->current<0){

					/* playing index reached 0, so stop
					playing this sample */

					vnf->current=0;
					vnf->active=0;
					break;
				}
			}
		}
		else{

			/* The sample is playing forward */

			if(vnf->islooping){

				/* the sample is looping, so check if
					it reached the loopend index */

				if(vnf->current>idxlend){
		
					if(vnf->isbidi){

						/* sample is doing bidirectional loops, so 'bounce'
						the current index against the idxlend */

						vnf->isreverse=1;
						vnf->increment=-vnf->increment;
						vnf->current=idxlend-(vnf->current-idxlend); /* ?? */
					}
					else{
						/* normal backwards looping, so set the
							current position to loopend index */

						vnf->current=idxlpos+(vnf->current-idxlend);
					}
				}
			}
			else{

				/* sample is not looping, so check
					if it reached the last position */

				if(vnf->current>idxsize){

					/* yes, so stop playing this sample */

					vnf->current=0;
					vnf->active=0;
					break;
				}
			}
		}

		/* Vraag een far ptr op van het sampleadres
			op byte offset vnf->current, en hoeveel samples
			daarvan geldig zijn (VOORDAT segment overschrijding optreed) */

		if(!(s=Samples[vnf->samplehandle])){
			vnf->current=0;
			vnf->active=0;
			break;
		}

		SLONG end;
	
		if(vnf->isreverse)
			end = (vnf->islooping) ? idxlpos : 0;
		else
			end = (vnf->islooping) ? idxlend : idxsize;

		/* Als de sample simpelweg niet beschikbaar is, of als
			sample gestopt moet worden sample stilleggen en stoppen */
		/* mix 'em: */

        UWORD done=NewPredict(vnf->current,end,vnf->increment,todo);

        if(!done){
            vnf->active=0;
            break;
        }

        if((mode & DMODE_NOCLICK) && vnf->fadecount){

            /* limit the number of samples to mix to the fadecount: */

            if(done>vnf->fadecount) done=vnf->fadecount;

#ifndef DUMMYMIX
            if(vnf->flags&SF_16BITS){
                if(mode & DMODE_STEREO) Mix16StereoNoclick((SWORD *)s,ptr,vnf->current,vnf->increment,done,vnf->olvolmul,vnf->orvolmul,vnf->lvolinc,vnf->rvolinc);
                else                    Mix16MonoNoclick((SWORD *)s,ptr,vnf->current,vnf->increment,done,vnf->olvolmul,vnf->lvolinc);
            }
            else{
                if(mode & DMODE_STEREO) Mix08StereoNoclick((SBYTE *)s,ptr,vnf->current,vnf->increment,done,vnf->olvolmul,vnf->orvolmul,vnf->lvolinc,vnf->rvolinc);
                else                    Mix08MonoNoclick((SBYTE *)s,ptr,vnf->current,vnf->increment,done,vnf->olvolmul,vnf->lvolinc);
            }
#endif
            // advance the volume pointer

            vnf->olvolmul+=(vnf->lvolinc*done);
            vnf->orvolmul+=(vnf->rvolinc*done);

            vnf->fadecount-=done;

            if(!vnf->fadecount){
                vnf->olvolmul=vnf->lvolmul;
                vnf->orvolmul=vnf->rvolmul;
                vnf->lvolinc=0;
                vnf->rvolinc=0;
            }
        }
        else{
            /* normal clicky mode.. don't mix anything at zero volume */

			if(vnf->volume){
#ifndef DUMMYMIX
				if(vnf->flags&SF_16BITS){
					if(mode&DMODE_STEREO){
						if(mode&DMODE_INTERP) Mix16StereoInterp((SWORD *)s,ptr,vnf->current,vnf->increment,done,vnf->lvolmul,vnf->rvolmul);
						else                  Mix16StereoNormal((SWORD *)s,ptr,vnf->current,vnf->increment,done,vnf->lvolmul,vnf->rvolmul);
					}
					else{
						if(mode&DMODE_INTERP) Mix16MonoInterp((SWORD *)s,ptr,vnf->current,vnf->increment,done,vnf->lvolmul);
						else                  Mix16MonoNormal((SWORD *)s,ptr,vnf->current,vnf->increment,done,vnf->lvolmul);
					}
				}
				else{
					if(mode&DMODE_STEREO){
						if(mode&DMODE_INTERP) Mix08StereoInterp((SBYTE *)s,ptr,vnf->current,vnf->increment,done,vnf->lvolmul,vnf->rvolmul);
						else                  Mix08StereoNormal((SBYTE *)s,ptr,vnf->current,vnf->increment,done,vnf->lvolmul,vnf->rvolmul);
					}
					else{
						if(mode&DMODE_INTERP) Mix08MonoInterp((SBYTE *)s,ptr,vnf->current,vnf->increment,done,vnf->lvolmul);
						else                  Mix08MonoNormal((SBYTE *)s,ptr,vnf->current,vnf->increment,done,vnf->lvolmul);
					}
				}
#endif
			}
		}

		// advance sample pointer

		vnf->current+=(vnf->increment*done);

		todo-=done;
		ptr+=(mode & DMODE_STEREO) ? (done<<1) : done;
	}
}


void MDRIVER_MXX::WritePortion(SBYTE *buf,int todo)
/*
	Mixes 'todo' samples to 'buf'.. The number of samples has
	to fit into the tickbuffer.
*/
{
	int t;

	/* clear the mixing buffer: */

	memset(tickbuffer,0,(mode & DMODE_STEREO) ? todo<<3 : todo<<2);

	/* add each channel one-by-one */

	if(mode & DMODE_NOCLICK){
		for(t=0;t<channels;t++){
			if(dying[t].active && dying[t].fadecount) AddChannel(&dying[t],todo);
		}
	}

	for(t=0;t<channels;t++){
		if(channel[t].active) AddChannel(&channel[t],todo);
	}

	if(mode & DMODE_16BITS)
		Sample32To16Copy(tickbuffer,(SWORD *)buf,(mode & DMODE_STEREO) ? todo<<1 : todo,16-ampshift);
	else
		Sample32To8Copy(tickbuffer,buf,(mode & DMODE_STEREO) ? todo<<1 : todo,24-ampshift);
}



void MDRIVER_MXX::VOICEPARMS::NewParms()
{
	int pan,vol,lvol,rvol;

	// kick channel ?

	if(kick){
		current=(start << FRACBITS);
		active=1;
		kick=0;
		olvolmul=0;
		orvolmul=0;
	}

	// frequency sanity check

	if(frequency<1000) active=0;

	// compute other parameters if channel is active
				
	if(active){
		increment=fraction2long(frequency,(UWORD)parent->frequency);

		if(isreverse) increment=-increment;

		vol=volume;
		pan=panning;

		if(parent->mode & DMODE_STEREO){

			if(flags & SF_SURROUND){
				// surround, so no panning allowed
				lvolmul=(parent->maxvol*vol)/510;
				rvolmul=-lvolmul;
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

				lvolmul=(parent->maxvol*lvol)/255;
				rvolmul=(parent->maxvol*rvol)/255;
			}
		}
		else{
			lvolmul=(parent->maxvol*vol)/255;
			rvolmul=lvolmul;
		}
		
		/* fade to a new volume in 32 steps */

		fadecount=(UWORD)(parent->frequency/689);
		lvolinc=(lvolmul-olvolmul)/fadecount;
		rvolinc=(rvolmul-orvolmul)/fadecount;
	}
}



void MDRIVER_MXX::WriteSamples(SBYTE *buffer,int todo)
{
	int t,part;

	while(todo>0){

		if(tickleft==0){

			/* preserve channels */

			if(mode & DMODE_NOCLICK) for(t=0;t<channels;t++) dying[t]=channel[t];

			/* call tickhandler routine, if present */
			
			Tick();

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

		buffer+=samples2bytes(part);
	}
}



int MDRIVER_MXX::WriteBytes(SBYTE *buffer,int todo)
{
	todo=bytes2samples(todo);
	WriteSamples(buffer,todo);
	return samples2bytes(todo);
}


void MDRIVER_MXX::WriteSilentSamples(SBYTE *buffer,int todo)
{
}


int MDRIVER_MXX::WriteSilentBytes(SBYTE *buffer,int todo)
{
	if(mode&DMODE_16BITS){
		memset(buffer,0,todo);
	}
	else{
		memset(buffer,0x80,todo);
	}
	return todo;
}


void MDRIVER_MXX::VoiceSetVolume(int voice,UBYTE volume)
{	
	if(voice<0) return;
	channel[voice].volume=volume;
}



void MDRIVER_MXX::VoiceSetPanning(int voice,UBYTE panning)
{	
	if(voice<0) return;
	channel[voice].panning=panning;
}



void MDRIVER_MXX::VoiceSetFrequency(int voice,ULONG frequency)
{	
	if(voice<0) return;
	channel[voice].frequency=frequency;
}



void MDRIVER_MXX::VoiceStop(int voice)
{	
	if(voice<0) return;
	channel[voice].kick=0;
	channel[voice].active=0;
}



int MDRIVER_MXX::VoiceActive(int voice)
{	
	if(voice<0) return 0;
	return channel[voice].active;
}



void MDRIVER_MXX::VoiceKeyOff(int voice)
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



void MDRIVER_MXX::VoiceKeyOn(int voice)
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



void MDRIVER_MXX::VoicePlay(int voice,int handle,ULONG start,ULONG size,ULONG reppos,ULONG repend,ULONG sreppos,ULONG srepend,UWORD flags)
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

