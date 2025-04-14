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

File:           MMOD_MD2.CPP
Description:    -
Version:        1.00 - original
                1.01 - v0.91 fixed pattern delay 

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mmod_mod.h"


static UBYTE VibratoTable[32]={
    0,24,49,74,97,120,141,161,
    180,197,212,224,235,244,250,253,
    255,253,250,244,235,224,212,197,
    180,161,141,120,97,74,49,24
};


static UWORD npertab[16][60]={

// -> Tuning 0

    1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,906,
    856,808,762,720,678,640,604,570,538,508,480,453,
    428,404,381,360,339,320,302,285,269,254,240,226,
    214,202,190,180,170,160,151,143,135,127,120,113,
    107,101,95,90,85,80,75,71,67,63,60,56,

// -> Tuning 1

    1700,1604,1514,1430,1348,1274,1202,1134,1070,1010,954,900,
    850,802,757,715,674,637,601,567,535,505,477,450,
    425,401,379,357,337,318,300,284,268,253,239,225,
    213,201,189,179,169,159,150,142,134,126,119,113,
    106,100,94,89,84,79,75,71,67,63,59,56,

// -> Tuning 2

    1688,1592,1504,1418,1340,1264,1194,1126,1064,1004,948,894,
    844,796,752,709,670,632,597,563,532,502,474,447,
    422,398,376,355,335,316,298,282,266,251,237,224,
    211,199,188,177,167,158,149,141,133,125,118,112,
    105,99,94,88,83,79,74,70,66,62,59,56,

// -> Tuning 3

    1676,1582,1492,1408,1330,1256,1184,1118,1056,996,940,888,
    838,791,746,704,665,628,592,559,528,498,470,444,
    419,395,373,352,332,314,296,280,264,249,235,222,
    209,198,187,176,166,157,148,140,132,125,118,111,
    104,99,93,88,83,78,74,70,66,62,59,55,

// -> Tuning 4

    1664,1570,1482,1398,1320,1246,1176,1110,1048,990,934,882,
    832,785,741,699,660,623,588,555,524,495,467,441,
    416,392,370,350,330,312,294,278,262,247,233,220,
    208,196,185,175,165,156,147,139,131,124,117,110,
    104,98,92,87,82,78,73,69,65,62,58,55,

// -> Tuning 5

    1652,1558,1472,1388,1310,1238,1168,1102,1040,982,926,874,
    826,779,736,694,655,619,584,551,520,491,463,437,
    413,390,368,347,328,309,292,276,260,245,232,219,
    206,195,184,174,164,155,146,138,130,123,116,109,
    103,97,92,87,82,77,73,69,65,61,58,54,

// -> Tuning 6

    1640,1548,1460,1378,1302,1228,1160,1094,1032,974,920,868,
    820,774,730,689,651,614,580,547,516,487,460,434,
    410,387,365,345,325,307,290,274,258,244,230,217,
    205,193,183,172,163,154,145,137,129,122,115,109,
    102,96,91,86,81,77,72,68,64,61,57,54,

// -> Tuning 7

    1628,1536,1450,1368,1292,1220,1150,1086,1026,968,914,862,
    814,768,725,684,646,610,575,543,513,484,457,431,
    407,384,363,342,323,305,288,272,256,242,228,216,
    204,192,181,171,161,152,144,136,128,121,114,108,
    102,96,90,85,80,76,72,68,64,60,57,54,

// -> Tuning -8

    1814,1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,
    907,856,808,762,720,678,640,604,570,538,508,480,
    453,428,404,381,360,339,320,302,285,269,254,240,
    226,214,202,190,180,170,160,151,143,135,127,120,
    113,107,101,95,90,85,80,75,71,67,63,60,

// -> Tuning -7

    1800,1700,1604,1514,1430,1350,1272,1202,1134,1070,1010,954,
    900,850,802,757,715,675,636,601,567,535,505,477,
    450,425,401,379,357,337,318,300,284,268,253,238,
    225,212,200,189,179,169,159,150,142,134,126,119,
    112,106,100,94,89,84,79,75,71,67,63,59,

// -> Tuning -6

    1788,1688,1592,1504,1418,1340,1264,1194,1126,1064,1004,948,
    894,844,796,752,709,670,632,597,563,532,502,474,
    447,422,398,376,355,335,316,298,282,266,251,237,
    223,211,199,188,177,167,158,149,141,133,125,118,
    111,105,99,94,88,83,79,74,70,66,62,59,

// -> Tuning -5

    1774,1676,1582,1492,1408,1330,1256,1184,1118,1056,996,940,
    887,838,791,746,704,665,628,592,559,528,498,470,
    444,419,395,373,352,332,314,296,280,264,249,235,
    222,209,198,187,176,166,157,148,140,132,125,118,
    111,104,99,93,88,83,78,74,70,66,62,59,

// -> Tuning -4

    1762,1664,1570,1482,1398,1320,1246,1176,1110,1048,988,934,
    881,832,785,741,699,660,623,588,555,524,494,467,
    441,416,392,370,350,330,312,294,278,262,247,233,
    220,208,196,185,175,165,156,147,139,131,123,117,
    110,104,98,92,87,82,78,73,69,65,61,58,

// -> Tuning -3

    1750,1652,1558,1472,1388,1310,1238,1168,1102,1040,982,926,
    875,826,779,736,694,655,619,584,551,520,491,463,
    437,413,390,368,347,328,309,292,276,260,245,232,
    219,206,195,184,174,164,155,146,138,130,123,116,
    109,103,97,92,87,82,77,73,69,65,61,58,

// -> Tuning -2

    1736,1640,1548,1460,1378,1302,1228,1160,1094,1032,974,920,
    868,820,774,730,689,651,614,580,547,516,487,460,
    434,410,387,365,345,325,307,290,274,258,244,230,
    217,205,193,183,172,163,154,145,137,129,122,115,
    108,102,96,91,86,81,77,72,68,64,61,57,

// -> Tuning -1

    1724,1628,1536,1450,1368,1292,1220,1150,1086,1026,968,914,
    862,814,768,725,684,646,610,575,543,513,484,457,
    431,407,384,363,342,323,305,288,272,256,242,228,
    216,203,192,181,171,161,152,144,136,128,121,114,
    108,101,96,90,85,80,76,72,68,64,60,57
};


void MMODULE_MOD::AUDTMP::DoVibrato()
{
    UBYTE q;
    UWORD temp;

    q=(vibpos>>2)&0x1f;

    switch(wavecontrol&3){

        case 0: // sine
            temp=VibratoTable[q];
            break;

        case 1: // ramp down
            q<<=3;
            if(vibpos<0) q=255-q;
            temp=q;
            break;

        case 2: // square wave
            temp=255;
            break;
    }

    temp*=vibdepth;
    temp>>=7;

    if(vibpos>=0)
        period=tmpperiod+temp;
    else
        period=tmpperiod-temp;

    if(tick) vibpos+=vibspd;    // do not update when vbtick==0
}


void MMODULE_MOD::AUDTMP::DoTremolo()
{
    UBYTE q;
    UWORD temp;

    q=(trmpos>>2)&0x1f;

    switch((wavecontrol>>4)&3){

        case 0: // sine
            temp=VibratoTable[q];
            break;

        case 1: // ramp down
            q<<=3;
            if(trmpos<0)    q=255-q;
            temp=q;
            break;

        case 2: // square wave
            temp=255;
            break;
    }

    temp*=trmdepth;
    temp>>=6;

    if(trmpos>=0){
        volume=tmpvolume+temp;
        if(volume>64) volume=64;
    }
    else{
        volume=tmpvolume-temp;
        if(volume<0) volume=0;
    }

    if(tick) trmpos+=trmspd;    // do not update when vbtick==0
}



void MMODULE_MOD::AUDTMP::DoToneSlide()
{
    int dist;

    if(!tick){      // do not update when vbtick==0
        tmpperiod=period;
        return;
    }

    /* We have to slide a->tmpperiod towards a->wantedperiod, so
       compute the difference between those two values */

    dist=period-wantedperiod;

    if( dist==0 ||                  // if they are equal
        portspeed>abs(dist) ){      // or if portamentospeed is too big

        period=wantedperiod;        // make tmpperiod equal tperiod
    }
    else if(dist>0){                // dist>0 ?
        period-=portspeed;          // then slide up
    }
    else
        period+=portspeed;          // dist<0 -> slide down

//  if(glissando){

        /* If glissando is on, find the nearest
           halfnote to a->tmpperiod */

//      for(t=0;t<60;t++){
//          if(tmpperiod>=npertab[finetune][t]) break;
//      }

//      period=npertab[finetune][t];
//  }
//  else
    tmpperiod=period;
}


void MMODULE_MOD::AUDTMP::DoVolSlide(UBYTE dat)
{
    if(!tick) return;               // do not update when vbtick==0
    tmpvolume+=(dat>>4);            // volume slide
    tmpvolume-=(dat&0xf);
    if(tmpvolume<0) tmpvolume=0;
    if(tmpvolume>64) tmpvolume=64;
}



void MMODULE_MOD::AUDTMP::HandleEEffects(UBYTE hi,UBYTE lo)
{
    switch(hi){

        case 0x0:   // filter toggle, not supported
            break;

        case 0x1:   // fineslide up
            if(!tick) tmpperiod-=lo;
            break;

        case 0x2:   // fineslide dn
            if(!tick) tmpperiod+=lo;
            break;

        case 0x3:   // glissando ctrl
            glissando=lo;
            break;

        case 0x4:   // set vibrato waveform
            wavecontrol&=0xf0;
            wavecontrol|=lo;
            break;

        case 0x5:   // set finetune
            if(!tick){
                finetune=lo;
                tmpperiod=npertab[lo][note];
            }
            break;

        case 0x6:   // set patternloop
            if(!tick){
                    
                /* hmm.. this one is a real kludge. But now it
                   works. */

                if(lo){     // set reppos or repcnt ?

                    /* set repcnt, so check if repcnt already is set,
                       which means we are already looping */

                    if(parent->loopcnt>0)
                        parent->loopcnt--;      // already looping, decrease counter
                    else
                        parent->loopcnt=lo;     // not yet looping, so set repcnt

                    if(parent->loopcnt)         // jump to reppos if repcnt>0
                        parent->row=parent->looppos;
                }
                else{
                    parent->looppos=parent->row-1;  // set reppos
                }
            }
            break;

        case 0x7:   // set tremolo waveform
            wavecontrol&=0x0f;
            wavecontrol|=lo<<4;
            break;

        case 0x8:   // not used
            break;

        case 0x9:   // retrig note

            /* only retrigger if
               data nibble > 0 */

            if(lo>0){
                if(retrig==0){

                    /* when retrig counter reaches 0,
                       reset counter and restart the sample */

                    kick=1;
                    retrig=lo;
                }
                retrig--; // countdown
            }
            break;

        case 0xa:   // fine volume slide up
            if(!tick){
                tmpvolume+=lo;
                if(tmpvolume>64) tmpvolume=64;
            }
            break;

        case 0xb:   // fine volume slide dn
            if(!tick){
                tmpvolume-=lo;
                if(tmpvolume<0) tmpvolume=0;
            }
            break;

        case 0xc:   // cut note

            /* When vbtick reaches the cut-note value,
               turn the volume to zero ( Just like
               on the amiga) */

            if(tick>=lo) tmpvolume=0;           // just turn the volume down
            break;

        case 0xd:   // note delay

            /* delay the start of the
               sample until vbtick==nib */

            if(tick==lo){
                kick=1;
            }
            else{
                kick=0;
            }
            break;

        case 0xe:   // pattern delay
            if(!tick){
                if(!parent->patdly2) parent->patdly=lo+1;
            }
            break;

        case 0xf:   // invert loop, not supported
            break;
    }
}


void MMODULE_MOD::AUDTMP::HandleEffects(UBYTE eff,UBYTE dat)
{
    UBYTE hi,lo;

    hi=dat>>4;
    lo=dat&0xf;

    ownper=0;
    ownvol=0;

    switch(eff){

        case 0x0:   // arpeggio
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

                period=npertab[finetune&0xf][tnote];
                ownper=1;
            }
            break;

        case 0x1:   // portamento up
            if(dat!=0) slidespeed=dat;
            if(tick) tmpperiod-=slidespeed;
            break;

        case 0x2:   // portamento dn
            if(dat!=0) slidespeed=dat;
            if(tick) tmpperiod+=slidespeed;
            break;

        case 0x3:   // toneportamento (toneslide)
            kick=0;
            if(dat!=0) portspeed=dat;
            DoToneSlide();
            ownper=1;
            break;

        case 0x4:   // vibrato
            if(dat&0x0f) vibdepth=dat&0xf;
            if(dat&0xf0) vibspd=(dat&0xf0)>>2;
            DoVibrato();
            ownper=1;
            break;

        case 0x5:   // tone+volume slide
            DoToneSlide();
            DoVolSlide(dat);
            ownper=1;
            break;

        case 0x6:   // vibrato + volslide
            DoVibrato();
            DoVolSlide(dat);
            ownper=1;
            break;

        case 0x7:
            if(dat&0x0f) trmdepth=dat&0xf;
            if(dat&0xf0) trmspd=(dat&0xf0)>>2;
            DoTremolo();
            ownvol=1;
            break;

        case 0x8:
            panning=dat;
            break;

        case 0x9:
            if(dat) soffset=(UWORD)dat<<8;       /* <- 0.43 fix.. */
            start=soffset;
//          if(start>a->s->length) a->start=a->s->length;
            break;

        case 0xa:
            DoVolSlide(dat);
            break;

        case 0xb:
            if(parent->patdly2) break;
            parent->breakpos=0;
            parent->posjump=dat+1;
            break;

        case 0xc:
            if(tick) break;
            if(dat>64) dat=64;
            tmpvolume=dat;
            break;

        case 0xd:
            if(parent->patdly2) break;
            parent->breakpos=1+(hi*10)+lo;
            break;

        case 0xe:
            HandleEEffects(hi,lo);
            break;

        case 0xf:
            if(tick || parent->patdly2) break;

            if(/* mp_extspd && */ dat>=0x20){
                parent->bpm=dat;
            }
            else{
                if(dat){                        /* <- v0.44 bugfix */
                    parent->speed=dat;
                    parent->tick=tick=0;
                }
            }
            break;
    }

    if(!ownper) period=tmpperiod;
    if(!ownvol) volume=tmpvolume;
}


void MMODULE_MOD::AUDTMP::HandleTick(UBYTE nte,UBYTE ins,UBYTE eff,UBYTE dat)
{
    tick=parent->tick;

    if(tick==0 && !parent->patdly2){

        if(ins!=0){             // instrument change ?
            ins--;              // yes, so put all instrument values into aud
            sample=ins;
            handle=parent->samples[sample].handle;
            volume=tmpvolume=parent->samples[sample].volume;
//          aud->size=pf->samples[inst].length;
            finetune=parent->samples[sample].finetune;
            retrig=0;
        }

        if(nte!=0){
            nte--;
            note=nte;

            UWORD localperiod=npertab[parent->samples[sample].finetune&0xf][note];
            wantedperiod=localperiod;
            tmpperiod=localperiod;

//          if(eff!=0x3 && eff!=0x5){
//              tmpperiod=period;
                kick=1;
                start=0;
//          }

            // retrig tremolo and vibrato waves ?

            if(!(wavecontrol&0x80)) trmpos=0;
            if(!(wavecontrol&0x08)) vibpos=0;
        }
    }

    HandleEffects(eff,dat);
}


