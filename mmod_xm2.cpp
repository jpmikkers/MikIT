/*

File:           MMOD_XM2.CPP
Description:    -
Version:        1.00 - original
                1.01 - v0.91 a note off with an instrument value behind it should result in a note-off anyway
                1.02 - 24 May 1998 patched HandleTick EDx behaviour (the volume column override didn't work with 
				       a note delay, since the volume would only be set at tick 0)
				       
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mmod_xm.h"

#define LIMIT(x,lo,hi) if(x<lo) x=lo; else if(x>hi) x=hi;


static UBYTE VibratoTable[32]={
	0,24,49,74,97,120,141,161,
	180,197,212,224,235,244,250,253,
	255,253,250,244,235,224,212,197,
	180,161,141,120,97,74,49,24
};



void MMODULE_XM::AUDT::SetVolume(SBYTE ivol)
{
	LIMIT(ivol,0,64);
	volume=ivol;
	realvolume=ivol;
}


void MMODULE_XM::AUDT::ChangeVolume(SBYTE ivol)
{
	LIMIT(ivol,0,64);
	realvolume=ivol;
}


void MMODULE_XM::AUDT::SetPeriod(UWORD iperiod)
{
	LIMIT(iperiod,40,50000);
	realperiod=iperiod;
	period=iperiod;
}


void MMODULE_XM::AUDT::ChangePeriod(UWORD iperiod)
{
	LIMIT(iperiod,40,50000);
	realperiod=iperiod;
}



void MMODULE_XM::AUDT::KeyOn()
{
	SetVolume(s->volume);
	panning=s->panning;
	venvpos=0;
	penvpos=0;
	avibpos=0;
	aswppos=0;
	fadevol=32768;
	keyon=1;
	tremorcnt=0;
	mtrigcnt=0;
	/* reset vibpos ? */
	if(!(vibctl&4)) vibpos=0;
	/* reset trmpos ? */
	if(!(trmctl&4)) trmpos=0;
}


void MMODULE_XM::AUDT::DoVibrato()
{
	UBYTE q;
	UWORD temp;

	q=vibpos&0x1f;

	switch(vibctl&3){

		case 0: /* sine */
			temp=VibratoTable[q];
			break;

		case 1: /* ramp down */
			q<<=3;
			temp=(vibpos & 32) ? 255-q : q;
			break;

		case 2: /* square wave */
			temp=255;
			break;
	}

	temp*=vibdpt;
	temp>>=7;
	temp<<=2;

	if(vibpos&32)
		ChangePeriod(period-temp);
	else
		ChangePeriod(period+temp);

	if(tick) vibpos=(vibpos+vibspd) & 63;
}



void MMODULE_XM::AUDT::DoNoteSlide()
{
	UWORD slsp=(UWORD)noteslspd<<2;

	if(destperiod){

		UWORD tper=period,delta;

		if(destperiod==tper) return;

		if(tper<destperiod){
			delta=destperiod-tper;
			tper+=(slsp>delta) ? delta : slsp;
		}
		else{
			delta=tper-destperiod;
			tper-=(slsp>delta) ? delta : slsp;
		}

		SetPeriod(tper);
	}
}


void MMODULE_XM::AUDT::DoVolSlide()
{
	UBYTE dat=volslspd;
	if(dat&0xf0) dat&=0xf0;
	SetVolume(volume + (dat>>4) - (dat&0xf));
}


void MMODULE_XM::AUDT::DoTremolo()
{
	UBYTE q;
	UWORD temp;

	q=trmpos&0x1f;

	switch(trmctl&3){

		case 0: /* sine */
			temp=VibratoTable[q];
			break;

		case 1: /* ramp down */
			q<<=3;
			temp=(trmpos & 32) ? 255-q : q;
			break;

		case 2: /* square wave */
			temp=255;
			break;
	}

	temp*=trmdpt;
	temp>>=6;

	if(trmpos&32)
		ChangeVolume(volume-temp);
	else
		ChangeVolume(volume+temp);

	trmpos=(trmpos+trmspd) & 63;
}


void MMODULE_XM::AUDT::DoMultiRetrig()
{
	if(mtrigspd==0) return;

	mtrigcnt++;

	if(mtrigcnt>=mtrigspd){
		kick=1;
		mtrigcnt=0;

		switch(mtrigfad){

			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				SetVolume(volume-(1<<(mtrigfad-1)));
				break;

			case 6:
				SetVolume((2*volume)/3);
				break;

			case 7:
				SetVolume(volume>>1);
				break;

			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
				SetVolume(volume+(1<<(mtrigfad-1)));
				break;

			case 14:
				SetVolume((3*volume)/2);
				break;

			case 15:
				SetVolume(volume<<1);
				break;
		}
	}
}


void MMODULE_XM::AUDT::HandleVolEffects(UBYTE vol)
{
	if(vol>=0x10 && vol<=0x50){
		if(tick==fetch) SetVolume(vol-0x10);
	}
	else{
		UBYTE dat=vol&0xf;

		switch(vol>>4){

			case 0x6:					/* volslide down */
				SetVolume(volume-dat);
				break;

			case 0x7:					/* volslide up */
				SetVolume(volume+dat);
				break;

			/* volume-row fine volume slide is compatible with protracker
			   EBx and EAx effects i.e. a zero nibble means DO NOT SLIDE, as
			   opposed to 'take the last sliding value'.
			*/

			case 0x8:						/* finevol down */
				if(!tick) SetVolume(volume-dat);
				break;

			case 0x9:                       /* finevol up */
				if(!tick) SetVolume(volume+dat);
				break;

			case 0xa:                       /* set vibrato speed */
				if(dat) vibspd=dat;
				break;

			case 0xb:						/* vibrato (+ depth) */
				if(tick){
					if(dat) vibdpt=dat;
				}
				DoVibrato();
				break;

			case 0xc:                       /* set panning */
				if(!tick) panning=(dat<<4);
				break;

			case 0xd:                       /* panning slide left */
				/* only slide when data nibble not zero: */
				if(tick){
					if(dat){
						if(dat < panning) panning-=dat;
						else panning=0;
					}
					else panning=0;  /* <-- !!! fastracker BUG I think */
				}
				break;

			case 0xe:                       /* panning slide right */
				/* only slide when data nibble not zero: */
				if(tick){
					if(dat){
						if(dat < (255-panning)) panning+=dat;
						else panning=255;
					}
				}
				break;

			case 0xf:                       /* tone porta */
				/* don't start the sample: */

                kick=0;

                /* 0x3 gets it's sliding speed at tick 0.. I don't
                   know if it also does this at the other ticks */

                if(dat) noteslspd=dat<<4;
                if(tick) DoNoteSlide();
                break;
        }
    }
}


void MMODULE_XM::AUDT::HandleEEffects(UBYTE eff,UBYTE dat)
{
    switch(eff){

        case 0x1: /* 0xE1: fine porta up */
            if(!tick){
                if(dat) fportupspd=dat; else dat=fportupspd;
                SetPeriod(period-((UWORD)dat<<2));
            }
            break;

        case 0x2: /* 0xE2: fine porta down */
            if(!tick){
                if(dat) fportdnspd=dat; else dat=fportdnspd;
                SetPeriod(period+((UWORD)dat<<2));
            }
            break;

        case 0x3: /* 0xE3: gliss control.. blech! */
            break;

        case 0x4: /* 0xE4: set vibrato control */
            vibctl=dat;
            break;

/*      case 0x5: 0xe5 set finetune
        already taken care of in the main note/instrument fetch */

        case 0x6: /* 0xE6: set begin/loop */
            if(!tick){
                if(dat==0)
                    parent->looppos=parent->row;
                else{
                    if(parent->loopcnt>0){
                        parent->loopcnt--;
                        if(parent->loopcnt>0){
                            parent->breakpos=parent->looppos+1;
                            if(!parent->posjump) parent->posjump=parent->songpos+1;
                        }
                    }
                    else{
                        parent->loopcnt=dat;
                        parent->breakpos=parent->looppos+1;
                        if(!parent->posjump) parent->posjump=parent->songpos+1;
                    }
                }
            }
            break;

        case 0x7: /* 0xE7: set tremolo control */
            trmctl=dat;
            break;

        case 0x9: /* 0xE9: retrig note */
            if(dat==0){
                /* only start sample again at tick 0 if dat is zero */
                if(!tick) kick=1;
            }
            else{
                if(tick && (tick % dat)==0){
                    UBYTE tmp;
                    kick=1;
                    tmp=volume;
                    KeyOn();
                    SetVolume(tmp);
                }
            }
            break;

        case 0xA: /* 0xEA: finevol up */
            if(!tick){
                if(dat) fvolupspd=dat; else dat=fvolupspd;
                SetVolume(volume+dat);
            }
            break;

        case 0xB: /* 0xEB: finevol down */
            if(!tick){
                if(dat) fvoldnspd=dat; else dat=fvoldnspd;
                SetVolume(volume-dat);
            }
            break;

        case 0xC: /* 0xEC: notecut */
            if(tick>=dat) SetVolume(0); /* just turn the volume down */
            break;

/*      case 0xd: 0xed note delay
         already taken care of in the main note/instrument fetch */

        case 0xE: /* 0xEE: pattern delay */
            parent->patdelayd=dat;
            break;
    }
}



void MMODULE_XM::AUDT::HandleStdEffects(UBYTE vol,UBYTE eff,UBYTE dat)
{
    UBYTE hi=dat>>4,lo=dat&0xf;

    switch(eff){

        case 0x0: /* effect 0: arpeggio */
            if(dat!=0){
                UBYTE tnote=note;

                switch(tick%3){
                    case 1:
                        tnote+=hi;
                        break;

                    case 2:
                        tnote+=lo;
                        break;
                }

                ChangePeriod(parent->getperiod(tnote+s->relnote,finetune));
                perdropback=1;
            }
            break;

        case 0x1: /* effect 1: slide up */
            if(tick){
                if(dat) portupspd=dat; else dat=portupspd;
                SetPeriod(period-((UWORD)dat<<2));
            }
            break;

        case 0x2: /* effect 2: slide down */
            if(tick){
                if(dat) portdnspd=dat; else dat=portdnspd;
                SetPeriod(period+((UWORD)dat<<2));
            }
            break;

        case 0x3: /* effect 3: noteslide */

            /* don't start the sample: */

			kick=0;

			/* 0x3 gets it's sliding speed at tick 0.. I don't
			   know if it also does this at the other ticks */

			if(dat) noteslspd=dat;
			if(tick) DoNoteSlide();
			break;

		case 0x4: /* effect 4: vibrato */

			/* don't fetch any new parameters at tick 0: */

            if(tick){
                if(hi) vibspd=hi;
                if(lo) vibdpt=lo;
            }
            DoVibrato();
            perdropback=1;
            break;

        case 0x5: /* effect 5: noteslide + volume slide */
            if(tick){
                if(dat) volslspd=dat;
                DoNoteSlide();
                DoVolSlide();
            }
            break;

        case 0x6: /* effect 6: vibrato + volume slide */
            if(tick){
                if(dat) volslspd=dat;
                DoVolSlide();
            }
            DoVibrato();
            perdropback=1;
            break;

        case 0x7: /* effect 7: tremolo */

            /* don't process it at all at tick 0 */

			if(tick){
				if(hi) trmspd=hi;
				if(lo) trmdpt=lo;
				DoTremolo();
			}
			break;

		case 0x8: /* effect 8: set panning position */
			if(!tick) panning=dat;
			break;

		case 0x9: /* effect 9: set sample start position */
			if(!tick){
				if(dat) startpos=0x100*dat;
				altstart=1;
			}
			break;

		case 0xa: /* effect A: volume slide */
			if(tick){
				if(dat) volslspd=dat;
				DoVolSlide();
			}
			break;

		case 0xb: /* effect B: position jump */
			if(!tick){
				parent->breakpos=0;	/* clear pending pattern breaks */
				parent->posjump=1+dat;
			}
			break;

		case 0xc: /* effect C: set volume */
			if(!tick) SetVolume(dat);
			break;

		case 0xd: /* effect D: pattern break */
			if(!tick){
				parent->breakpos=(hi*10)+lo+1;
				parent->posjump=parent->songpos+2;
			}
			break;

		case 0xe:
			HandleEEffects(hi,lo);
			break;

		case 0xf: /* effect F: set speed/bpm */
			if(dat>=0x20){
				parent->bpm=dat;
			}
			else{
				parent->speed=dat;
			}
			break;

		case 'G'-55: /* set global volume */
			parent->globalvolume=(dat>64) ? 64 : dat;
			break;

		case 'H'-55: /* global volumeslide */
			if(tick){
				if(dat) parent->globalslide=dat; else dat=parent->globalslide;
				if(dat&0xf0) dat&=0xf0;
				parent->globalvolume=parent->globalvolume+(dat>>4)-(dat&0xf);
				LIMIT(parent->globalvolume,0,64);
			}
			break;

		case 'K'-55: /* key off */
			if(tick>=dat) keyon=0;
			break;

		case 'L'-55: /* set envelope position */
			if(!tick){
				UWORD points;

				points=i->volenv.points-1;
				venvpos=(dat>points) ? points : dat;

				points=i->panenv.points-1;
				penvpos=(dat>points) ? points : dat;
			}
			break;

		case 'P'-55: /* panning slide */
			if(tick){
				if(dat) panslspd=dat; else dat=panslspd;

				hi=dat>>4;
				lo=dat&0xf;

				if(hi){
					if(hi < (255-panning)) panning+=hi;
					else panning=255;
				}
				else{
					if(lo < panning) panning-=lo;
					else panning=0;
				}
			}
			break;

		case 'R'-55: /* multi retrig note */
			if(!tick){
				if(hi) mtrigfad=hi;
                if(lo) mtrigspd=lo;
				if(vol<0x10) DoMultiRetrig();
			}
			else DoMultiRetrig();
			break;

		case 'T'-55: /* tremor */
			if(tick){
				UBYTE on,off;
				if(dat) tremorspd=dat; else dat=tremorspd;

				on=(dat>>4)+1;
				off=(dat&0xf)+1;

				tremorcnt%=(on+off);
				ChangeVolume((tremorcnt<on) ? volume : 0);
				tremorcnt++;
			}
			break;

		case 'X'-55: /* extra fine slide */
			if(!tick){
				if(hi==1){
					if(lo) ffportupspd=lo; else lo=ffportupspd;
					SetPeriod(period-lo);
				}
				else if(hi==2){
					if(lo) ffportdnspd=lo; else lo=ffportdnspd;
					SetPeriod(period+lo);
				}
			}
			break;

		case 'Z'-55: /* set filter */
			if(!tick){
				if(dat<128){
					filtercutoff=dat;
				}
				else{
					filterdamping=dat-128;
				}
			}
			break;
	}
}




void MMODULE_XM::AUDT::HandleTick(UBYTE nte,UBYTE ins,UBYTE vol,UBYTE eff,UBYTE dat)
{
	tick=parent->tick;
	altstart=0;

	/* determine the tick at which we
	   should get a new note & instrument */

	fetch=(eff==0xe && (dat&0xf0)==0xd0) ? dat&0xf : 0;

	if(tick==0 && parent->patdelayc){
		/* no new note at tick 0 when patdelay is active */
	}
	else if(tick==fetch){

//		printf("%2.2d -",note);

		/*
			New instrument? -> get it NOW, because if there's a new
            note it has to determine the correct sample from this NEW
            instrument.
        */

        if(ins) i=parent->GetInstrument(ins-1);

        /* new note? -> process it */

        if(nte){
            nte--;
            if(nte<96){
                /* normal note */

                note=nte;       /* store it (for arpeggio) */

                if(eff!=3 && eff!=5 && (vol&0xf0)!=0xf0){
                    s=parent->GetSample(i,nte);
                    finetune=s->finetune;
                    /* override default sample finetune with effect 0xE5? */
                    if(eff==0xe && (dat&0xf0)==0x50) finetune=((SBYTE)dat-8)<<4;

                    SetPeriod(parent->getperiod(nte+s->relnote,finetune));
                    kick=1;
                }
                else{
                    destperiod=parent->getperiod(nte+s->relnote,finetune);
                }
            }
            else{
                /* key off */
                keyon=0;
                ins=0;          /* BUGFIX 0.91: <- set ins to zero or else the following line will continue the sound anyway */
            }
        }

        /*
            Now do the rest of the new instrument stuff, because this
            stuff is dependant on the new sample (set by the note)
        */

        if(ins) KeyOn();
    }

    HandleVolEffects(vol);
    HandleStdEffects(vol,eff,dat);
}
