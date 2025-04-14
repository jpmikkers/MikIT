/*

File:           MMOD_S32.CPP
Description:    -
Version:        1.00 - original

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mmod_s3m.h"


static SBYTE VibTables[3][256]={

{
       0,  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
      24, 26, 27, 29, 30, 32, 33, 34, 36, 37, 38, 39, 41, 42, 43, 44,
      45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
      59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 60, 60,
      59, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,
      45, 44, 43, 42, 41, 39, 38, 37, 36, 34, 33, 32, 30, 29, 27, 26,
      24, 23, 22, 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2,
       0, -2, -3, -5, -6, -8, -9,-11,-12,-14,-16,-17,-19,-20,-22,-23,
     -24,-26,-27,-29,-30,-32,-33,-34,-36,-37,-38,-39,-41,-42,-43,-44,
     -45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,-56,-57,-58,-59,
     -59,-60,-60,-61,-61,-62,-62,-62,-63,-63,-63,-64,-64,-64,-64,-64,
     -64,-64,-64,-64,-64,-64,-63,-63,-63,-62,-62,-62,-61,-61,-60,-60,
     -59,-59,-58,-57,-56,-56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,
     -45,-44,-43,-42,-41,-39,-38,-37,-36,-34,-33,-32,-30,-29,-27,-26,
     -24,-23,-22,-20,-19,-17,-16,-14,-12,-11, -9, -8, -6, -5, -3, -2
},

{
     -64,-63,-63,-62,-62,-61,-61,-60,-60,-59,-59,-58,-58,-57,-57,-56,
     -56,-55,-55,-54,-54,-53,-53,-52,-52,-51,-51,-50,-50,-49,-49,-48,
     -48,-47,-47,-46,-46,-45,-45,-44,-44,-43,-43,-42,-42,-41,-41,-40,
     -40,-39,-39,-38,-38,-37,-37,-36,-36,-35,-35,-34,-34,-33,-33,-32,
     -32,-31,-31,-30,-30,-29,-29,-28,-28,-27,-27,-26,-26,-25,-25,-24,
     -24,-23,-23,-22,-22,-21,-21,-20,-20,-19,-19,-18,-18,-17,-17,-16,
     -16,-15,-15,-14,-14,-13,-13,-12,-12,-11,-11,-10,-10, -9, -9, -8,
      -8, -7, -7, -6, -6, -5, -5, -4, -4, -3, -3, -2, -2, -1, -1,  0,
       0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7,  8,
       8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16,
      16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24,
      24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 32,
      32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39, 40,
      40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48,
      48, 49, 49, 50, 50, 51, 51, 52, 52, 53, 53, 54, 54, 55, 55, 56,
      56, 57, 57, 58, 58, 59, 59, 60, 60, 61, 61, 62, 62, 63, 63, 64
},

{
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
}
};

static UWORD s3mperiods[12]={
    1712*16,1616*16,1524*16,1440*16,1356*16,1280*16,
    1208*16,1140*16,1076*16,1016*16,960*16,907*16
};


void MMODULE_S3M::AUDTMP::VolSlideUp(UBYTE v)
{
    if((tmpvolume+v)<64) tmpvolume+=v; else tmpvolume=64;
}


void MMODULE_S3M::AUDTMP::VolSlideDown(UBYTE v)
{
    if(tmpvolume>v) tmpvolume-=v; else tmpvolume=0;
}


void MMODULE_S3M::AUDTMP::DoVolSlide()
{
    UBYTE hi=volslidespd>>4;
    UBYTE lo=volslidespd&0xf;

    if(!tick){
        if(hi==0xf){
            if(lo) VolSlideDown(lo); else VolSlideUp(0xf);      // s3m compatibility slide 0xf0
        }
        else if(lo==0xf){
            if(hi) VolSlideUp(hi); else VolSlideDown(0xf);      // s3m compatibility slide 0x0f
        }
    }
    else{
        if(hi==0){
            VolSlideDown(lo);               
        }
        else if(lo==0){
            VolSlideUp(hi);
        }
    }
}


void MMODULE_S3M::AUDTMP::DoPitchSlideDown()
{
    UBYTE hi=pitchslidespd>>4;
    UBYTE lo=pitchslidespd&0xf;

    if(!tick){
        if(hi==0xf){
            tmpperiod+=lo<<2;
        }
        else if(hi==0xe){
            tmpperiod+=lo;
        }
    }
    else{
        if(hi<0xe) tmpperiod+=UWORD(pitchslidespd)<<2;
    }
}


void MMODULE_S3M::AUDTMP::DoPitchSlideUp()
{
    UBYTE hi=pitchslidespd>>4;
    UBYTE lo=pitchslidespd&0xf;

    if(!tick){
        if(hi==0xf){
            tmpperiod-=lo<<2;
        }
        else if(hi==0xe){
            tmpperiod-=lo;
        }
    }
    else{
        if(hi<0xe) tmpperiod-=UWORD(pitchslidespd)<<2;
    }
}


void MMODULE_S3M::AUDTMP::EffectG(UBYTE dat)
{
    if(!tick && dat){
        pitchslidespd=dat;
        pitchslidespd<<=2;
    }

    if(!wantedperiod) return;

    if(tick){
        SLONG dist=wantedperiod-tmpperiod;

        if(dist>=0){
            if(pitchslidespd>=dist){
                tmpperiod=wantedperiod;
            }
            else{
                tmpperiod+=pitchslidespd;
            }
        }
        else{
            if(pitchslidespd>=(-dist)){
                tmpperiod=wantedperiod;
            }
            else{
                tmpperiod-=pitchslidespd;
            }
        }
    }
}


void MMODULE_S3M::AUDTMP::DoVibrato(UBYTE vibspd,UBYTE vibdpt)
{
    int val;
    ownper=1;

    if((wavecontrol&3)<3){
        val=VibTables[wavecontrol&3][vibptr];
    }
    else{
        val=(rand()&127)-64;
    }

    val*=vibdpt;
    val>>=7;
    val<<=2;

    period=tmpperiod+val;

    if(tick) vibptr+=vibspd; // (vibspd<<2);        // old effects : only update vibrato on non-row frames
}




void MMODULE_S3M::AUDTMP::DoTremolo()
{
    int val;
    ownvol=1;

    if((wavecontrol>>4)<3){
        val=VibTables[wavecontrol>>4][trmptr];
    }
    else{
        val=(rand()&127)-64;
    }

    val*=trmdpt;
    val>>=7;
    val<<=2;

    volume=tmpvolume+val;

    if(volume>64) volume=64;
    else if(volume<0) volume=0;

    if(tick) trmptr+=trmspd; // (vibspd<<2);        // old effects : only update vibrato on non-row frames
}



void MMODULE_S3M::AUDTMP::DoS3MRetrig(UBYTE inf)
{
    UBYTE hi,lo;

    hi=inf>>4;
    lo=inf&0xf;

    if(lo){
        s3mrtgslide=hi;
        s3mrtgspeed=lo;
    }

    if(hi){
        s3mrtgslide=hi;
    }

    /* only retrigger if
       lo nibble > 0 */

    if(s3mrtgspeed>0){
        if(retrig==0){

            /* when retrig counter reaches 0,
               reset counter and restart the sample */

            kick=1;
            retrig=s3mrtgspeed;

            if(tick){                     /* don't slide on first retrig */
				
				int val=tmpvolume;
				
				switch(s3mrtgslide){

					case 1: val-=1; break;
					case 2: val-=2; break;
					case 3: val-=4; break;
					case 4: val-=8; break;
					case 5: val-=16; break;
					case 6: val<<=1; val/=3; break;
					case 7: val>>=1; break;
					case 9: val+=1; break; 
					case 10: val+=2; break; 
					case 11: val+=4; break; 
					case 12: val+=8; break; 
					case 13: val+=16; break; 
					case 14: val*=3; val>>=1; break; 
					case 15: val<<=1; break; 
				}
				if(val<0) val=0;
				if(val>64) val=64;

				tmpvolume=val;
			}
		}
		retrig--; /* countdown */
	}
}


void MMODULE_S3M::AUDTMP::EffectSx(UBYTE dat)
{
	UBYTE nib;

	nib=dat&0xf;

	switch(dat>>4){

		case 0x0:       /* filter toggle, not supported */
				break;

		case 0x1:       /* glissando ctrl */
				glissando=nib;
				break;

		case 0x2:       /* set finetune */
/*              a->c2spd=finetune[nib]; */
/*              a->tmpperiod=GetPeriod(a->note,pf->samples[a->sample].transpose,a->c2spd); */
				break;

		case 0x3:       /* set vibrato waveform */
				wavecontrol&=0xf0;
				wavecontrol|=nib;
				break;

		case 0x4:       /* set tremolo waveform */
				wavecontrol&=0x0f;
				wavecontrol|=nib<<4;
				break;

		case 0x8:       /* set panpos */
				panning=nib<<4;
				break;

		case 0xB:       /* set patternloop */
				if(tick) break;

				/* hmm.. this one is a real kludge. But now it
				   works. */

				if(nib){                /* set reppos or repcnt ? */

					/* set repcnt, so check if repcnt already is set,
					   which means we are already looping */

					if(parent->repcnt>0)
						parent->repcnt--;               /* already looping, decrease counter */
					else
						parent->repcnt=nib;             /* not yet looping, so set repcnt */

					if(parent->repcnt)                      /* jump to reppos if repcnt>0 */
						parent->row=parent->reppos;
				}
				else{
					parent->reppos=parent->row-1;     /* set reppos */
				}
				break;

		case 0xC:       /* cut note */

				/* When vbtick reaches the cut-note value,
				   turn the volume to zero ( Just like
				   on the amiga) */

				if(tick>=nib){
					tmpvolume=0;                 /* just turn the volume down */
				}
				break;

		case 0xD:       /* note delay */

				/* delay the start of the
				   sample until vbtick==nib */

				if(tick==nib){
					kick=1;
				}
				else kick=0;
				break;

		case 0xE:       /* pattern delay */
				if(tick) break;
				if(!parent->patdly2) parent->patdly=nib+1;                   /* only once (when vbtick=0) */
				break;

	}
}



void MMODULE_S3M::AUDTMP::HandleEffects(UBYTE eff,UBYTE dat)
{
	ownper=0;
	ownvol=0;
	ownofs=0;

	switch(eff+'A'-1){

		case 'A':			// Axx set speed
			if(!tick && dat) parent->speed=dat;
			break;

		case 'B':			// Bxx Jump to order
			/* if(!tick) */ parent->posjump=dat+1;	
			break;

		case 'C':			// Cxx Break to row
			/* if(!tick) */ parent->breakpos=dat+1;
			break;

		case 'D':			// Dxx volume slide
			if(!tick && dat) volslidespd=dat;
			DoVolSlide();
			break;

		case 'E':			// Exx pitch slide down
			if(!tick && dat) pitchslidespd=dat;
			DoPitchSlideDown();
			break;

		case 'F':			// Fxx pitch slide up
			if(!tick && dat) pitchslidespd=dat;
			DoPitchSlideUp();
			break;

		case 'G':			// Gxx note portamento
			EffectG(dat);
			break;

		case 'H':			// Hxx vibrato
			if(!tick){
				if(dat&0x0f) vibdpt=dat&0xf;
				if(dat&0xf0) vibspd=dat>>4;
			}
			DoVibrato(vibspd<<2,vibdpt<<2);
			break;

		case 'I':			// Ixx tremor
			if(!tick && dat) tremorspd=dat;

			{
				UBYTE on,off;

				on=(tremorspd>>4)+1;
				off=(tremorspd&0xf)+1;

				s3mtremor%=(on+off);
				volume=(s3mtremor < on ) ? tmpvolume:0;
				s3mtremor++;
				ownvol=1;
			}
			break;

		case 'J':			// Jxx arpeggio
			{
				if(!tick && dat) arpeggio=dat;

				UBYTE tnote=(12*(note>>4))+(note&0xf);

				if(arpeggio!=0 && c2spd){

					switch(tick%3){
						case 1:
							tnote+=(arpeggio>>4); break;
						case 2:
							tnote+=(arpeggio&0xf); break;
					}

					period=UWORD(((8363L*s3mperiods[tnote%12]) >> (tnote/12) ) / c2spd);
					ownper=1;
				}
			}
			break;

		case 'K':			// Kxx dual command : H00 + Dxx
			if(!tick && dat) volslidespd=dat;
			DoVibrato(vibspd<<2,vibdpt<<2);
			DoVolSlide();
			break;

		case 'L':			// Lxx dual command : G00 + Dxx
			if(!tick && dat) volslidespd=dat;
			EffectG(0);
			DoVolSlide();
			break;

		case 'O':			// O set sample offset
			if(!tick && dat){
				startoffset=dat;  
				startoffset<<=8;
			}
			ownofs=1;
			break;

		case 'Q':			// Q retrig with volume modifier
			DoS3MRetrig(dat);
			break;

		case 'R':			// R tremolo
			if(!tick){
				if(dat&0x0f) trmdpt=dat&0x0f;
				if(dat&0xf0) trmspd=(dat&0xf0)>>2;
			}
			DoTremolo();
			break;

		case 'S':			// S special
			EffectSx(dat);
			break;

		case 'T':			// T set/slide tempo
			if(!tick && dat>=0x20) parent->bpm=dat;
			break;

		case 'U':			// U fine vibrato
			if(!tick){
				if(dat&0x0f) vibdpt=dat&0xf;
				if(dat&0xf0) vibspd=dat>>4;
			}
			DoVibrato(vibspd<<2,vibdpt);
			break;

		case 'V':			// V set global volume
			if(!tick && dat<=64){
				parent->globalvolume=dat;
			}
			break;
	}

	if(!ownper) period=tmpperiod;
	if(!ownvol) volume=tmpvolume;
}



void MMODULE_S3M::AUDTMP::HandleTick(UBYTE nte,UBYTE ins,UBYTE vol,UBYTE eff,UBYTE dat)
{
	tick=parent->tick;

	if(tick==0 && !parent->patdly2){

		if(ins>0 && ins<=parent->numsmp){		// instrument change ?
			sample=ins;
//			handle=parent->samples[sample-1].handle;
			c2spd=parent->samples[sample-1].c2spd;
			volume=tmpvolume=parent->samples[sample-1].volume;
//			aud->size=pf->samples[inst].length;
//			finetune=parent->samples[sample].finetune;
			retrig=0;
		}

		if(vol<=64){
			volume=tmpvolume=vol;
		}
		
		if(nte!=0xff){
			if(nte==0xfe){
				kick=0;
				cut=1;
			}
			else if(sample>0){

				note=nte;

				UWORD localperiod=UWORD(((8363L*s3mperiods[note&0xf]) >> (note>>4) ) / c2spd);
				wantedperiod=localperiod;

				if(eff!=7 && eff!=12){
					tmpperiod=localperiod;
					kick=1;
				}

				// retrig tremolo and vibrato waves ?

				if(!(wavecontrol&0x80)) trmptr=0;
				if(!(wavecontrol&0x08)) vibptr=0;
			}
		}
	}

	HandleEffects(eff,dat);
}

