/*

File:           MMOD_IT2.CPP
Description:    -
Version:        1.00 - original
                1.01 - high offset command SAx back in
                1.02 - fixed 'old effects' flag vibrato
                1.03 - v0.91 implemented surround effect S91
                1.04 - v0.91 fixed tremor effect Ixx
                1.05 - v0.91 fixed volcol effect Gx (use VolFxTable for speeds)
                1.06 - v0.91 fixed patternbreak and positionjump  Bxx and Cxx
                1.06 - v0.91 fixed patterndelay (in ticks) S6x
                1.07 - v0.99 REALLY fixed tremor Ixx .. differentiate between old and new effects (Hugo Visser)
                1.08 - v1.00b2 another tremor fix: check for zero on % operation
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mmod_it.h"

UBYTE MMODULE_IT::VolFxTable[10]={
    0, 1, 4, 8, 16, 32, 64, 96, 128, 255
};


void MMODULE_IT::ITCOLUMN::PanSlideLeft(UBYTE v)
{
    if(NP>v) NP-=v; else NP=0;
}


void MMODULE_IT::ITCOLUMN::PanSlideRight(UBYTE v)
{
    if((NP+v)<64) NP+=v; else NP=64;
}


void MMODULE_IT::ITCOLUMN::VolSlideUp(UBYTE v)
{
    if((vol+v)<64) vol+=v; else vol=64;
}


void MMODULE_IT::ITCOLUMN::VolSlideDown(UBYTE v)
{
    if(vol>v) vol-=v; else vol=0;
}


void MMODULE_IT::ITCOLUMN::FrqSlideDown(UBYTE v)
{
    if(m->Flags&8)
        frq=MulDiv(frq,LinearSlideDownTable[v],65536);
    else{
        /*
            Amigapitchslidedown:
            FinalFrequency = InitialFrequency * (1712*8363) / ((1712*8363) + SlideValue*InitialFrequency)
        */

        frq=MulDiv(frq,1712L*8363,(1712L*8363)+(frq*v*4));
    }
}


void MMODULE_IT::ITCOLUMN::FrqSlideUp(UBYTE v)
{
    if(m->Flags&8)
        frq=MulDiv(frq,LinearSlideUpTable[v],65536);
    else{
        /*
            Amigapitchslideup:
            FinalFrequency = InitialFrequency * (1712*8363) / ((1712*8363) -SlideValue*InitialFrequency)
        */

        frq=MulDiv(frq,1712L*8363,(1712L*8363)-(frq*v*4));
    }
}


void MMODULE_IT::ITCOLUMN::EffectA()
{
    if(!tick && n.dat) m->speed=n.dat;
}


void MMODULE_IT::ITCOLUMN::EffectB()
{
/*  if(!tick) */ m->orderjump=n.dat+1;
}


void MMODULE_IT::ITCOLUMN::EffectC()
{
/* if(!tick) */ m->breaktorow=n.dat+1;
}


void MMODULE_IT::ITCOLUMN::EffectD()
{
    if(!tick){
        if(n.dat) volslidespd=n.dat;
    }

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


void MMODULE_IT::ITCOLUMN::EffectE()
{
    if(!tick){
        if(n.dat) pitchslidespd=n.dat;
    }

    UBYTE hi=pitchslidespd>>4;
    UBYTE lo=pitchslidespd&0xf;

    if(!tick){
        if(hi==0xf){
            FrqSlideDown(lo);
        }
        else if(hi==0xe){
            frq=MulDiv(frq,FineLinearSlideDownTable[lo],65536);
        }
    }
    else{
        if(hi<0xe) FrqSlideDown(pitchslidespd);
    }
}



void MMODULE_IT::ITCOLUMN::EffectF()
{
    if(!tick){
        if(n.dat) pitchslidespd=n.dat;
    }

    UBYTE hi=pitchslidespd>>4;
    UBYTE lo=pitchslidespd&0xf;

    if(!tick){
        if(hi==0xf){
            FrqSlideUp(lo);
        }
        else if(hi==0xe){
            frq=MulDiv(frq,FineLinearSlideUpTable[lo],65536);
        }
    }
    else{
        if(hi<0xe) FrqSlideUp(pitchslidespd);
    }
}



void MMODULE_IT::ITCOLUMN::EffectG(UBYTE dat)
{
    if(!tick && dat) pitchslidespd=dat;

    if(!destfrq) return;

    if(tick){
        if(frq<destfrq){
            FrqSlideUp(pitchslidespd);
            if(frq>destfrq) frq=destfrq;
        }
        else if(frq>destfrq){
            FrqSlideDown(pitchslidespd);
            if(frq<destfrq) frq=destfrq;
        }
    }
}



void MMODULE_IT::ITCOLUMN::DoVibrato()
{
    int val;
    ownfrq=1;

    if(vibtyp<3){
        val=VibTables[vibtyp][vibptr];
    }
    else{
        val=(rand()&127)-64;
    }

    if(m->Flags&16){
        val*=vibdpt;
        val>>=7;
        val=-val;
    }
    else{
        val*=vibdpt;
        val>>=8;
    }

    ofrq=frq;

    if(val<0){
        if(m->Flags&8)
            ofrq=MulDiv(frq,LinearSlideDownTable[-val],65536);
        else
            ofrq=MulDiv(frq,1712L*8363,(1712L*8363)-(frq*val*4) );      // fix 0.08
    }
    else if(val>0){
        if(m->Flags&8)
            ofrq=MulDiv(frq,LinearSlideUpTable[val],65536);
        else
            ofrq=MulDiv(frq,1712L*8363,(1712L*8363)-(frq*val*4) );      // fix 0.08
    }

    if(m->Flags&16){                        // FIX 0.08 .. support old effects
        if(tick) vibptr+=(vibspd<<2);       // old effects : only update vibrato on non-row frames
    }
    else{
        vibptr+=(vibspd<<2);
    }
}



void MMODULE_IT::ITCOLUMN::EffectH()
{
    if(!tick){
        if(n.dat>>4) vibspd=n.dat>>4;
        if(n.dat&0xf) vibdpt=(n.dat&0xf)<<2;
    }
    DoVibrato();
}



void MMODULE_IT::ITCOLUMN::EffectI()
{
    if(!tick && n.dat) tremorspd=n.dat;

    UBYTE on=(tremorspd>>4);            // v0.91 fixed tremor 
    UBYTE off=(tremorspd&0xf);

    if(m->Flags&16){                    // v0.99 : REALLY fixed tremor.. differentiate between old and new effects
        on++;
        off++;
    }

    if((on+off)>0) tremorptr%=(on+off); else tremorptr=0;   // v1.00b2 : check for zero on %
    ovol=(tremorptr<on) ? vol:0;
    tremorptr++;
    ownvol=1;
}


void MMODULE_IT::ITCOLUMN::EffectJ()
{
    if(!tick && n.dat) arpeggio=n.dat;

    switch(tick%3){
        case 0:
            break;

        case 1:
            ofrq=MulDiv(frq,LinearSlideUpTable[16U*(arpeggio>>4)],65536);
            ownfrq=1;
            break;
        
        case 2:
            ofrq=MulDiv(frq,LinearSlideUpTable[16U*(arpeggio&0xf)],65536);
            ownfrq=1;
            break;
    }
}


void MMODULE_IT::ITCOLUMN::EffectM()
{
    if(!tick && n.dat<=0x40) CV=n.dat;
}


void MMODULE_IT::ITCOLUMN::EffectO()
{
    if(!tick && (n.dat || highoffset)){                 // FIX 0.07
        startoffset=highoffset;
        startoffset<<=8;
        startoffset+=n.dat;  
        startoffset<<=8;
    }
    ownofs=1;
}


void MMODULE_IT::ITCOLUMN::EffectN()
{
    if(!tick){
        if(n.dat) chvolslidespd=n.dat;
    }

    UBYTE hi=chvolslidespd>>4;
    UBYTE lo=chvolslidespd&0xf;

    if(!tick){
        if(hi==0xf){
            if(CV>lo) CV-=lo; else CV=0;
        }
        else if(lo==0xf){
            if((CV+hi)<64) CV+=hi; else CV=64;
        }
    }
    else{
        if(hi==0){
            if(CV>lo) CV-=lo; else CV=0;
        }
        else if(lo==0){
            if((CV+hi)<64) CV+=hi; else CV=64;
        }
    }
}


void MMODULE_IT::ITCOLUMN::EffectP()
{
    if(!tick && n.dat) panslidespd=n.dat;

    UBYTE hi=panslidespd>>4;
    UBYTE lo=panslidespd&0xf;

    if(!tick){
        if(hi==0xf){
            PanSlideRight(lo);
        }
        else if(lo==0xf){
            PanSlideLeft(hi);
        }
    }
    else{
        if(hi==0){
            PanSlideRight(lo);              
        }
        else if(lo==0){
            PanSlideLeft(hi);
        }
    }

    CP=NP;
}


void MMODULE_IT::ITCOLUMN::EffectQ()
{
    if(!tick && n.dat) qspeed=n.dat;

    UBYTE qtyp=qspeed>>4;
    UBYTE qspd=qspeed&0xf;

//  ownvol=1;
    int val=vol;

    if(qptr>=qspd){

        switch(qtyp){
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

        if(val<0) val=0; else if(val>64) val=64;
        qptr=0;
        kick=1;
        retrig=1;
    }

    qptr++;
    vol=val;
}


void MMODULE_IT::ITCOLUMN::EffectR()
{
    if(!tick){
        if(n.dat>>4) trmspd=n.dat>>4;
        if(n.dat&0xf) trmdpt=n.dat&0xf;
    }

    int val;
    ownvol=1;

    if(trmtyp<3){
        val=VibTables[trmtyp][trmptr];
    }
    else{
        val=(rand()&127)-64;
    }

    val*=trmdpt;
    val>>=5;

    val+=vol;

    if(val<0) val=0; else if(val>64) val=64;

    ovol=val;

    trmptr+=(trmspd<<2);
}


void MMODULE_IT::ITCOLUMN::EffectSB(UBYTE dat)
{
    if(!tick){
        if(dat==0){
            loopbackpoint=m->row;
        }
    }

    if(tick==m->speed-1 && dat>0){
        loopbackcount++;
        if(loopbackcount>dat){
            loopbackcount=0;
            loopbackpoint=m->row+1;
        }
        else{
            m->breaktorow=loopbackpoint+1;
            m->orderjump=m->songpos+1;
        }
    }
}


void MMODULE_IT::ITCOLUMN::EffectSx()
{
    if(n.dat) sfxdata=n.dat;

    UBYTE hi=sfxdata>>4;
    UBYTE lo=sfxdata&0xf;

    switch(hi){

        case 3:
            if(!tick && lo<4) vibtyp=lo;
            break;

        case 4:
            if(!tick && lo<4) trmtyp=lo;
            break;

        case 5:
//          if(!tick && lo<4) pnbtyp=lo;
            break;

        case 6:
            if(!tick) m->extraticks+=lo;        // patterndelay in ticks
            break;
        
        case 7:
            if(lo<3){
                for(int t=0;t<64;t++) m->virt[t].PastNoteAction(m->colno,lo);
            }
            break;

        case 8:
            EffectX(lo<<4);
            break;

        case 9:
            if(lo==1) surround=1;
            break;

        case 10:            // doesn't seem to work in Impulse tracker ?
			if(!tick) highoffset=lo;
			break;

		case 11:
			EffectSB(lo);
			break;

		case 12:
			if(tick>lo){
				/* note cut */
				if(v) v->cut=1;
			}
			break;

		case 14:
			if(!tick){
				if(!m->patterndelay){
					m->patterndelrq=lo+1;
				}
			}
			break;

		case 15:
			if(!tick) midiselector=lo;
			break;
	}
}


void MMODULE_IT::ITCOLUMN::PostEffectSx()
{
	UBYTE hi=n.dat>>4;
	UBYTE lo=n.dat&0xf;

	if(hi==7) switch(lo){
	
		case 3:
		case 4:
		case 5:
		case 6:
			if(m->Flags&4 && v) v->NNA=lo-3;
			break;

		case 7:
			if(v) v->volprc.Flg&=~1;
			break;

		case 8:
			if(v && v->volprc.Lst) v->volprc.Flg|=1;
			break;

		case 9:
			if(v) v->panprc.Flg&=~1;
			break;

		case 10:
			if(v && v->panprc.Lst) v->panprc.Flg|=1;
			break;

		case 11:
			if(v) v->ptcprc.Flg&=~1;
			break;

		case 12:
			if(v && v->ptcprc.Lst) v->ptcprc.Flg|=1;
			break;
	}
	
/*	switch(hi){
		case 7:
			if(lo>=3 && lo<=6 && m->Flags&4){
				if(v && !v->background) v->NNA=lo-3;
			}
			else if(lo==7){
			}
			else if(lo==8){
				if(v && !v->background) v->volprc.Flg|=1;
			}
			break;
	}
*/
}


void MMODULE_IT::ITCOLUMN::EffectT()
{
	if(!tick && n.dat) temposlidespd=n.dat;

	if(tick){
		UBYTE hi=temposlidespd>>4;
		UBYTE lo=temposlidespd&0xf;
		
		if(hi==0){
			if(m->tempo>(0x20+lo)) m->tempo-=lo; else m->tempo=0x20;
		}
		else if(hi==1){
			if(m->tempo<(0xff-lo)) m->tempo+=lo; else m->tempo=0xff;
		}
	}
	else{
		if(temposlidespd>=0x20) m->tempo=temposlidespd;
	}
}


void MMODULE_IT::ITCOLUMN::EffectU()
{
	if(!tick){
		if(n.dat>>4) vibspd=n.dat>>4;
		if(n.dat&0xf) vibdpt=n.dat&0xf;
	}
	DoVibrato();
}


void MMODULE_IT::ITCOLUMN::EffectV()
{
	if(!tick && n.dat<=0x80) m->runGV=n.dat;
}


void MMODULE_IT::ITCOLUMN::EffectW()
{
	if(!tick){
		if(n.dat) gvolslidespd=n.dat;
	}

	UBYTE hi=gvolslidespd>>4;
	UBYTE lo=gvolslidespd&0xf;

	if(!tick){
		if(hi==0xf){
			if(m->runGV>lo) m->runGV-=lo; else m->runGV=0;
		}
		else if(lo==0xf){
			if(m->runGV<(128-hi)) m->runGV+=hi; else m->runGV=128;
		}
	}
	else{
		if(hi==0){
			if(m->runGV>lo) m->runGV-=lo; else m->runGV=0;
		}
		else if(lo==0){
			if(m->runGV<(128-hi)) m->runGV+=hi; else m->runGV=128;
		}
	}
}


void MMODULE_IT::ITCOLUMN::EffectX(UBYTE dat)
{
	if(!tick){
		CP=NP=((UWORD)dat+2)>>2;
		surround=0;
	}
}


void MMODULE_IT::ITCOLUMN::EffectY()
{
}


void MMODULE_IT::ITCOLUMN::VEffectA(UBYTE dat)
{
	if(!tick){
		if(dat) vvolslidespd=dat;	
		VolSlideUp(vvolslidespd);
	}
}


void MMODULE_IT::ITCOLUMN::VEffectB(UBYTE dat)
{
	if(!tick){
		if(dat) vvolslidespd=dat;	
		VolSlideDown(vvolslidespd);
	}
}


void MMODULE_IT::ITCOLUMN::VEffectC(UBYTE dat)
{
	if(!tick && dat) vvolslidespd=dat;
	if(tick) VolSlideUp(vvolslidespd);
}


void MMODULE_IT::ITCOLUMN::VEffectD(UBYTE dat)
{
	if(!tick && dat) vvolslidespd=dat;
	if(tick) VolSlideDown(vvolslidespd);
}


void MMODULE_IT::ITCOLUMN::VEffectE(UBYTE dat)
{
	if(!tick && dat) vpitchslidespd=dat;
	if(tick) FrqSlideDown(vpitchslidespd<<2);
}


void MMODULE_IT::ITCOLUMN::VEffectF(UBYTE dat)
{
	if(!tick && dat) vpitchslidespd=dat;
	if(tick) FrqSlideUp(vpitchslidespd<<2);
}


void MMODULE_IT::ITCOLUMN::VEffectG(UBYTE dat)
{
	EffectG(VolFxTable[dat]);
}


void MMODULE_IT::ITCOLUMN::VEffectH(UBYTE dat)
{
	if(!tick && dat) vibdpt=dat<<2;
	DoVibrato();
}
