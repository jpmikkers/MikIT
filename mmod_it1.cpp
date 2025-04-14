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

File:           MMOD_IT1.CPP
Description:    -
Version:        1.00 - original
                1.01 - v0.91 implemented loading of old instrument envelopes
                1.02 - v0.92 implemented compressed delta samples
                1.03 - v1.00b2 make sure envelopes are disabled when there are NO envelope points
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mmod_it.h"

/******************************************************************************
* TABLES
******************************************************************************/

ULONG MMODULE_IT::PitchTable[10*12]={
   2048,    2170,    2299,    2435,    2580,    2734,
   2896,    3069,    3251,    3444,    3649,    3866,
   4096,    4340,    4598,    4871,    5161,    5468,
   5793,    6137,    6502,    6889,    7298,    7732,
   8192,    8679,    9195,    9742,   10321,   10935,
  11585,   12274,   13004,   13777,   14596,   15464,
  16384,   17358,   18390,   19484,   20643,   21870,
  23170,   24548,   26008,   27554,   29193,   30929,
  32768,   34716,   36781,   38968,   41285,   43740,
  46341,   49097,   52016,   55109,   58386,   61858,
  65536,   69433,   73562,   77936,   82570,   87480,
  92682,   98193,  104032,  110218,  116772,  123715,
 131072,  138866,  147123,  155872,  165140,  174960,
 185364,  196386,  208064,  220436,  233544,  247431,
 262144,  277732,  294247,  311744,  330281,  349920,
 370728,  392772,  416128,  440872,  467088,  494862,
 524288,  555464,  588493,  623487,  660561,  699841,
 741455,  785544,  832255,  881744,  934175,  989724,
1048576, 1110928, 1176987, 1246974, 1321123, 1399681,
1482910, 1571089, 1664511, 1763488, 1868350, 1979448
};

ULONG MMODULE_IT::FineLinearSlideUpTable[16]={
  65536,   65595,   65654,   65714,   65773,   65832,   65892,   65951,
  66011,   66071,   66130,   66190,   66250,   66309,   66369,   66429
};

ULONG MMODULE_IT::FineLinearSlideDownTable[16]={
  65535,   65477,   65418,   65359,   65300,   65241,   65182,   65359,
  65065,   65006,   64947,   64888,   64830,   64772,   64713,   64645
};

ULONG MMODULE_IT::LinearSlideUpTable[257]={
  65536,   65773,   66011,   66250,   66489,   66730,   66971,   67213,
  67456,   67700,   67945,   68191,   68438,   68685,   68933,   69183,
  69433,   69684,   69936,   70189,   70443,   70693,   70953,   71210,
  71468,   71726,   71985,   72246,   72507,   72769,   73032,   73297,
  73562,   73828,   74095,   73563,   74632,   74902,   75172,   75444,
  75717,   75991,   76266,   76542,   76819,   77096,   77375,   77655,
  77936,   78218,   78501,   78785,   79069,   79355,   79642,   79930,
  80220,   80510,   80801,   81093,   81386,   81681,   81976,   82273,
  82570,   82869,   83169,   83469,   83771,   84074,   84378,   84683,
  84990,   85297,   85606,   85915,   86226,   86538,   86851,   87165,
  87480,   87796,   88114,   88433,   88752,   89073,   89396,   89719,
  90043,   90369,   90696,   91024,   91353,   91684,   92015,   92348,
  92682,   93017,   93354,   93691,   94030,   94370,   94711,   95054,
  95398,   95743,   96089,   96436,   96784,   97135,   97487,   97839,
  98193,   98548,   98905,   99262,   99621,   99982,  100343,  100706,
 101070,  101436,  101803,  102171,  102540,  102911,  103283,  103657,
 104032,  104408,  104786,  105165,  105545,  105927,  106310,  106694,
 107080,  107468,  107856,  108246,  108638,  109031,  109425,  109821,
 110218,  110617,  111017,  111418,  111821,  112226,  112631,  113039,
 113453,  113858,  114270,  114683,  115098,  115514,  115932,  116351,
 116772,  117194,  117618,  118043,  118470,  118899,  119329,  119760,
 120194,  120628,  121065,  121502,  121942,  122383,  122825,  123270,
 123715,  124163,  124612,  125063,  125515,  125969,  126425,  126882,
 127341,  127801,  128263,  128727,  129193,  129660,  130129,  130600,
 131072,  131546,  132022,  132499,  132978,  133459,  133942,  134427,
 134913,  135399,  135890,  136382,  136875,  137370,  137867,  138366,
 138866,  139368,  139872,  140378,  140886,  141395,  141907,  142420,
 142935,  143452,  143971,  144491,  145014,  145539,  146065,  146593,
 147123,  147655,  148189,  148725,  149263,  149803,  150345,  150889,
 151434,  151982,  152532,  153083,  153637,  154193,  154750,  155310,
 155872,  156435,  157001,  156569,  158139,  158711,  159285,  159861,
 160439,  161019,  161602,  162186,  162773,  163361,  163952,  164545,
 165140
};


ULONG MMODULE_IT::LinearSlideDownTable[257]={
  65535,   65300,   65065,   64830,   64596,   64364,   64132,   63901,
  63670,   63441,   63212,   62984,   62757,   62531,   62306,   62081,
  61858,   61635,   61413,   61191,   60971,   60751,   60532,   60314,
  60097,   59880,   59664,   59449,   59235,   59022,   58809,   58597,
  58386,   58176,   57966,   57757,   57549,   57341,   57135,   56929,
  56724,   56519,   56316,   56113,   55911,   55709,   55508,   55308,
  55109,   54910,   54713,   54515,   54319,   54123,   53928,   53734,
  53540,   53347,   53155,   52963,   52773,   52582,   52393,   52204,
  52016,   51829,   51642,   51456,   51270,   51085,   50901,   50718,
  50535,   50353,   50172,   49991,   49811,   49631,   49452,   49274,
  49097,   48920,   48743,   48568,   48393,   48128,   48044,   47871,
  47699,   47527,   47356,   47185,   47015,   46846,   46677,   46509,
  46341,   46174,   46008,   45842,   45677,   45512,   45348,   45185,
  45022,   44859,   44698,   44537,   44376,   44216,   44057,   43898,
  43740,   43582,   43425,   43269,   43113,   42958,   42803,   42649,
  42495,   42342,   42189,   42037,   41886,   41735,   41584,   41434,
  41285,   41136,   40988,   10840,   40639,   40566,   40400,   40253,
  40110,   39965,   39821,   39678,   39535,   39392,   39250,   39109,
  38968,   38828,   38688,   38548,   38409,   38271,   38133,   37996,
  37859,   37722,   37586,   37451,   37316,   37181,   37047,   36914,
  36781,   36648,   36516,   36385,   36254,   36123,   35993,   35863,
  35734,   35605,   35477,   35349,   35221,   35095,   34968,   34842,
  34716,   34591,   34467,   34343,   34219,   34095,   33973,   33850,
  33728,   33607,   33486,   33365,   33245,   33125,   33005,   32887,
  32768,   32650,   32532,   32415,   32298,   32182,   32066,   31950,
  31835,   31720,   31606,   31492,   31379,   31266,   31153,   31041,
  30929,   30817,   30706,   30596,   30485,   30376,   30226,   30157,
  30048,   29940,   29832,   29725,   29618,   29511,   29405,   29299,
  29193,   29088,   28983,   28879,   28774,   28671,   28567,   28464,
  28362,   28260,   28158,   28056,   27955,   27855,   27754,   27654,
  27554,   27455,   27356,   27258,   27159,   27062,   26964,   26867,
  26770,   26674,   26577,   26482,   26386,   26291,   26196,   26102,
  26008
};

SBYTE MMODULE_IT::VibTables[3][256]={
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
{64, 63, 63, 62, 62, 61, 61, 60, 60, 59, 59, 58, 58, 57, 57, 56,
56, 55, 55, 54, 54, 53, 53, 52, 52, 51, 51, 50, 50, 49, 49, 48,
48, 47, 47, 46, 46, 45, 45, 44, 44, 43, 43, 42, 42, 41, 41, 40,
40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32,
32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24,
24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,
8,  7,  7,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  0,
0, -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6, -7, -7, -8,
-8, -9, -9,-10,-10,-11,-11,-12,-12,-13,-13,-14,-14,-15,-15,-16,
-16,-17,-17,-18,-18,-19,-19,-20,-20,-21,-21,-22,-22,-23,-23,-24,
-24,-25,-25,-26,-26,-27,-27,-28,-28,-29,-29,-30,-30,-31,-31,-32,
-32,-33,-33,-34,-34,-35,-35,-36,-36,-37,-37,-38,-38,-39,-39,-40,
-40,-41,-41,-42,-42,-43,-43,-44,-44,-45,-45,-46,-46,-47,-47,-48,
-48,-49,-49,-50,-50,-51,-51,-52,-52,-53,-53,-54,-54,-55,-55,-56,
-56,-57,-57,-58,-58,-59,-59,-60,-60,-61,-61,-62,-62,-63,-63,-64
},
{64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
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
    
/******************************************************************************
* ITPROCESS
******************************************************************************/

void MMODULE_IT::ITPROCESS::Start(ITENVELOPE *e)
{
    Flg=e->Flg;
    
    // get number of ticks

    if(e->Num) Lst=e->points[e->Num-1].tick; else Lst=0;

    // loop ?

    if(Flg&2){
        LpB=e->points[e->LpB].tick;
        LpE=e->points[e->LpE].tick;
    }

    // sustain loop?

    if(Flg&4){
        SLB=e->points[e->SLB].tick;
        SLE=e->points[e->SLE].tick;
    }

    tick=0;
}


MMODULE_IT::ITPROCESS::ITPROCESS()
{
    Lst=0;
    Flg=0;
    tick=0;
}


UWORD MMODULE_IT::ITPROCESS::GetTick()
{
    return (Flg&1) ? tick : -1;
}


void MMODULE_IT::ITPROCESS::Process(int keyon)
{
    if(Flg&1){
        tick++;

        if(keyon && Flg&4){
            if(tick>SLE) tick=SLB;
        }
        else if(Flg & 2){
            if(tick>LpE) tick=LpB;
        }

        if(tick>Lst){
            tick=Lst;
            Flg|=64;
        }
    }
}


int MMODULE_IT::ITPROCESS::Done()
{
    return(Flg&64);
}


/******************************************************************************
* ITENVELOPE
******************************************************************************/


SWORD MMODULE_IT::ITENVELOPE::Interpolate(SWORD p,SWORD p1,SWORD p2,SWORD v1,SWORD v2)
{
    SWORD dp,dv,di;

    if(p1==p2) return v1;

    dv=v2-v1;
    dp=p2-p1;
    di=p-p1;

    return v1 + ((SLONG)(di*dv) / dp);
}


MMODULE_IT::ITENVELOPE::ITENVELOPE()
{
    Flg=0;
    Num=0;
    LpB=0;
    LpE=0;
    SLB=0;
    SLE=0;
}


void MMODULE_IT::ITENVELOPE::Load(MINPUT *in)
{
    Flg=in->read_UBYTE();
    Num=in->read_UBYTE();
    LpB=in->read_UBYTE();
    LpE=in->read_UBYTE();
    SLB=in->read_UBYTE();
    SLE=in->read_UBYTE();

//  printf("%d points\n",Num);

    for(int t=0;t<25;t++){
        points[t].value=in->read_UBYTE();
        points[t].tick=in->read_UWORD();
    }

    in->read_UBYTE();

    if(Num==0) Flg=0;       // 1.00b2: disable envelopes if there are no points

    Expand();
}


void MMODULE_IT::ITENVELOPE::LoadOld(MINPUT *in,UBYTE iFlg,UBYTE iLpB,UBYTE iLpE,UBYTE iSLB,UBYTE iSLE)
{
    int t;

    Flg=iFlg;
//  Num=in->read_UBYTE();
    LpB=iLpB;
    LpE=iLpE;
    SLB=iSLB;
    SLE=iSLE;

    in->seek(200,SEEK_CUR);     // skip 200 bytes

    for(t=0;t<25;t++){
        points[t].tick=in->read_UBYTE();
        points[t].value=in->read_UBYTE();
        if(points[t].value==0xff || points[t].tick==0xff) break;
    }

    Num=t;
//  printf("%d points\n",Num);
    if(Num==0) Flg=0;               // 1.00b2: disable envelopes if there are no points

    Expand();
}


/******************************************************************************
* ITVOLENV
******************************************************************************/

void MMODULE_IT::ITVOLENV::Expand()
{
    if(!Num) return;

    SWORD numvalues=points[Num-1].tick+1;

//  printf("%d volenvelope points\n",numvalues);

    values=new UBYTE[numvalues];

    int p1=0;
    int p2=1;

    for(SWORD t=0;t<numvalues;t++){
        SWORD t1=points[p1].tick;
        SWORD t2=points[p2].tick;
        UBYTE v1=points[p1].value;
        UBYTE v2=points[p2].value;

        values[t]=(UBYTE)Interpolate(t,t1,t2,v1,v2);

//      printf("%d,%d\n",t,values[t]);

        if(t==t2){
            p1++;
            p2++;
        }
    }
}


MMODULE_IT::ITVOLENV::ITVOLENV() : ITENVELOPE()
{
    values=NULL;
}


MMODULE_IT::ITVOLENV::~ITVOLENV()
{
    delete[] values;
}


/******************************************************************************
* ITPANENV
******************************************************************************/

void MMODULE_IT::ITPANENV::Expand()
{
//  printf("panenv flag %d, %d num\n",Flg,Num);
    if(!Num) return;

    SWORD numvalues=points[Num-1].tick+1;

//  printf("%d panenvelope points\n",numvalues);

    values=new SBYTE[numvalues];

    int p1=0;
    int p2=1;

    for(SWORD t=0;t<numvalues;t++){
        SWORD t1=points[p1].tick;
        SWORD t2=points[p2].tick;
        SBYTE v1=points[p1].value;
        SBYTE v2=points[p2].value;

        values[t]=(SBYTE)Interpolate(t,t1,t2,v1,v2);

//      printf("%d,%d\n",t,values[t]);

        if(t==t2){
            p1++;
            p2++;
        }
    }
}


MMODULE_IT::ITPANENV::ITPANENV() : ITENVELOPE()
{
//  puts("panenv constructed");
    values=NULL;
}


MMODULE_IT::ITPANENV::~ITPANENV()
{
    delete[] values;
}

/******************************************************************************
* ITPTCENV
******************************************************************************/

void MMODULE_IT::ITPTCENV::Expand()
{
    if(!Num) return;

    SWORD numvalues=points[Num-1].tick+1;
//  printf("%d ptcenvelope points\n",numvalues);

    values=new SWORD[numvalues];

    int p1=0;
    int p2=1;

    for(SWORD t=0;t<numvalues;t++){
        SWORD t1=points[p1].tick;
        SWORD t2=points[p2].tick;
        SWORD v1=(SBYTE)points[p1].value;
        SWORD v2=(SBYTE)points[p2].value;

        values[t]=Interpolate(t,t1,t2,v1<<3,v2<<3);

        if(t==t2){
            p1++;
            p2++;
        }
    }
}


MMODULE_IT::ITPTCENV::ITPTCENV() : ITENVELOPE()
{
    values=NULL;
}


MMODULE_IT::ITPTCENV::~ITPTCENV()
{
    delete[] values;
}


/******************************************************************************
* ITNOTE
******************************************************************************/


MMODULE_IT::ITNOTE::ITNOTE()
{
    msk=0;
    nte=0;
    ins=0;
    vol=0;
    eff=0;
    dat=0;
}


void MMODULE_IT::ITNOTE::EZDecode(UBYTE * &p)
/*
    Decodes one full note-tupel (msk,not,ins,vol,eff,dat). Assumes p is pointing at the mask byte.
*/
{
    msk=*p++;

    if(msk&1) nte=*p++;
    if(msk&2) ins=*p++;
    if(msk&4) vol=*p++;

    if(msk&8){
        eff=*p++;
        dat=*p++;
    }
}


void MMODULE_IT::ITNOTE::ITDecode(UBYTE * &p,UBYTE chv,ITNOTE &previous)
/*
    Decodes one full IT note-tupel (msk,not,ins,vol,eff,dat) based on the channelvariable (chv), the compressed data (p) 
    and the previous tupel values for this channel (previous). 
*/
{
    *this=previous;             // assume all tupel values are equal to the previous ones by default

    if(chv&128){
        msk=*p++;
        previous.msk=msk;
    }

    if(msk&1){
        nte=*p++;
        previous.nte=nte;
    }
            
    if(msk&2){
        ins=*p++;
        previous.ins=ins;
    }
            
    if(msk&4){
        vol=*p++;
        previous.vol=vol;
    }
            
    if(msk&8){
        eff=*p++;
        dat=*p++;
        previous.eff=eff;
        previous.dat=dat;
    }

    // recode 'msk' so it doesn't have any stupid previous bits

	msk=(msk&0xf)|(msk>>4);
}



int MMODULE_IT::ITNOTE::EZCount()
/*
	Returns the number of bytes this note-tupel will occupy in my own easy pattern format.
*/
{
	int result=0;
	if(msk){
		result+=2;					// +2 for channel number and mask bytes
		if(msk&0x1) result++;		// +1 for note byte
		if(msk&0x2) result++;		// +1 for ins byte
		if(msk&0x4) result++;		// +1 for vol byte
		if(msk&0x8) result+=2;		// +2 for eff/dat bytes
	}
	return result;
}



void MMODULE_IT::ITNOTE::EZEncode(UBYTE * &d,int channel)
{
	if(msk){						// new information present?
		*d++=channel+1;				// write channelno. + 1
		*d++=msk;					// write mask
				
		if(msk&1) *d++=nte;			// write note				
		if(msk&2) *d++=ins;			// write ins
		if(msk&4) *d++=vol;			// write vol
				
		if(msk&8){
			*d++=eff;				// write eff
			*d++=dat;				// write dat
		}
	}
}


/******************************************************************************
* ITSAMPLE
******************************************************************************/

MMODULE_IT::ITSAMPLE::ITSAMPLE()
{
	handle=-1;
	msample=NULL;
}



MMODULE_IT::ITSAMPLE::~ITSAMPLE()
{
//	if(handle>=0) driver->SampleFree(handle);
//	driver->SampleFree2(handle);
	delete msample;
}


void MMODULE_IT::ITSAMPLE::Load(MINPUT *in)
{
	UBYTE id[4];

    in->read_UBYTES(id,4);
    if(memcmp(id,"IMPS",4)) THROW MikModException("Not a sample");

    in->read_UBYTES(filename,13);
    GvL=in->read_UBYTE();
    Flg=in->read_UBYTE();
    Vol=in->read_UBYTE();
    in->read_STRING(smpname,26);
    Cvt=in->read_UBYTE();
	DfP=in->read_UBYTE();
    Length=in->read_ULONG();
    LoopBeg=in->read_ULONG();
    LoopEnd=in->read_ULONG();
    C5Speed=in->read_ULONG();
    SLoopBeg=in->read_ULONG();
    SLoopEnd=in->read_ULONG();
    SamplePtr=in->read_ULONG();
    ViS=in->read_UBYTE();
    ViD=in->read_UBYTE();
    ViR=in->read_UBYTE();
    ViT=in->read_UBYTE();
    
//  printf("Sample %s,Flag 0x%x, size %ld, sloopbeg %ld, sloopend %ld\n",smpname,Flg,Length,SLoopBeg,SLoopEnd);

	// assume there's no sample to begin with

    handle=-1;

    // sample data present?

    if(Flg & 1){
        // seek to raw sample data
        
        in->seek(SamplePtr,SEEK_SET);
        
//      printf("Seeked to %ld\n",SamplePtr);

        mmFlg=0;

        if(Flg & (1<<1)) mmFlg|=SF_16BITS;

/* stereo samples are ignored - they are in fact mono samples that once were stereo :)
        if(Flg & (1<<2)) THROW MikModException("Stereo samples not supportede");
*/      
        if(Flg & (1<<3)) mmFlg|=SF_ITCMP;           // v0.91 : new IT compression support
        if(Flg & (1<<4)) mmFlg|=SF_LOOP;
        if(Flg & (1<<5)) mmFlg|=SF_SUSLOOP; 
        if(Flg & (1<<6)) mmFlg|=SF_BIDI;
        if(Flg & (1<<7)) mmFlg|=SF_SUSBIDI;
        if(Cvt & 1)      mmFlg|=SF_SIGNED;
        if(Cvt & 4)      mmFlg|=SF_ITCDL;
        
        msample=new MSAMPLE(in,Length,LoopBeg,LoopEnd,mmFlg);
//      handle=driver->SampleLoad(in,Length,LoopBeg,LoopEnd,mmFlg);
//      handle=driver->SampleLoad2(msample);
    }
}






/******************************************************************************
* ITINSTRUMENT
******************************************************************************/

void MMODULE_IT::ITINSTRUMENT::Load(MINPUT *in)
{
        int t;
        UBYTE id[4];

        in->read_UBYTES(id,4);
        if(memcmp(id,"IMPI",4)) THROW MikModException("Not an instrument");

        in->read_UBYTES(filename,13);
        NNA=in->read_UBYTE();
        DCT=in->read_UBYTE();
        DCA=in->read_UBYTE();
        FadeOut=in->read_UWORD();
        PPS=in->read_UBYTE();
        PPC=in->read_UBYTE();
        GbV=in->read_UBYTE();
        DfP=in->read_UBYTE();
        RV=in->read_UBYTE();
        RP=in->read_UBYTE();
        TrkVers=in->read_UWORD();
        NoS=in->read_UBYTE();
        in->seek(1,SEEK_CUR);            // skip (x)
        in->read_STRING(insname,26);
        
        defaultcutoff=in->read_UBYTE();
        defaultdamping=in->read_UBYTE();

        in->seek(4,SEEK_CUR);            // skip 4 * x

//      printf("RV: %d RP: %d\n",RV,RP);
//      printf("DC: %d DD: %d\n",defaultcutoff,defaultdamping);
                
        for(t=0;t<120;t++){
                notesample[t].note=in->read_UBYTE();
                notesample[t].sample=in->read_UBYTE();
        }

        volenv.Load(in);
        panenv.Load(in);
        ptcenv.Load(in);

//        printf("%s\n",insname);
}       


void MMODULE_IT::ITINSTRUMENT::LoadOld(MINPUT *in)
{
        int t;
        UBYTE id[4];
        UBYTE Flg,VLS,VLE,SLS,SLE;

        in->read_UBYTES(id,4);
        if(memcmp(id,"IMPI",4)) THROW MikModException("Not an instrument");

        in->read_UBYTES(filename,13);

        Flg=in->read_UBYTE();
        VLS=in->read_UBYTE();
        VLE=in->read_UBYTE();
        SLS=in->read_UBYTE();
        SLE=in->read_UBYTE();
        in->seek(2,SEEK_CUR);            // skip 2

        FadeOut=in->read_UWORD()<<1;
        NNA=in->read_UBYTE();
        DCT=0;  in->read_UBYTE();
        DCA=0;
        TrkVers=in->read_UWORD();
        NoS=in->read_UBYTE();

        in->seek(1,SEEK_CUR);            // skip 1

        in->read_STRING(insname,26);

//      printf("%s\n",insname);

        in->seek(6,SEEK_CUR);            // skip 6

        for(t=0;t<120;t++){
                notesample[t].note=in->read_UBYTE();
                notesample[t].sample=in->read_UBYTE();
        }

//      DCA=in->read_UBYTE();
//      FadeOut=in->read_UWORD();
        PPS=0;
        PPC=60;
        GbV=128;
        DfP=128;
        RV=0;
        RP=0;

/*      printf("Old envelope:\n");

        for(t=0;t<200;t++){
            printf("%02x ",in->read_UBYTE());
        }

        printf("\nOld nodes:\n");

        for(t=0;t<50;t++){
            printf("%02x ",in->read_UBYTE());
        }
*/
    
        volenv.LoadOld(in,Flg,VLS,VLE,SLS,SLE);

//        volenv.Load(in);
//        panenv.Load(in);
//        ptcenv.Load(in);

//        printf("%s\n",insname);
}       


void MMODULE_IT::ITINSTRUMENT::CreateFromSample(int smpno)
{
    filename[0]=0;
    NNA=0;
    DCT=0;
    DCA=0;
    FadeOut=0;
    PPS=0;
    PPC=60;
    GbV=128;
    DfP=128;
    RV=0;
    RP=0;

    TrkVers=0;
    NoS=1;
    insname[0]=0;

    for(int t=0;t<120;t++){
        notesample[t].note=t;
        notesample[t].sample=smpno;
    }

    volenv.Flg=0;
    volenv.Num=0;

    panenv.Flg=0;
    panenv.Num=0;
    
    ptcenv.Flg=0;
    ptcenv.Num=0;
}       


UBYTE MMODULE_IT::ITINSTRUMENT::GetGbV()
{
    int r=((rand() & 255) - 128);   // r = -128 - +127
    r*=RV;                          // r = -12800 - 12700
    r>>=6;                          // r = -200 - +198
    r++;                            // r = -199 - +199
    r*=GbV;                         // r = -199*128 - +199*128
    r/=199;                         // r = -128 - +128
    r+=GbV;                         
    if(r<0) r=0; else if(r>128) r=128;
    return (UBYTE)r;
}


UBYTE MMODULE_IT::ITINSTRUMENT::RandomPan(UBYTE pan)
{
    int r=((rand() & 255) - 128);   // r = -128 - +127
    r*=RP;                          // r = -128*64 - +127*64
    r>>=7;
    r+=pan;
    if(r<0) r=0; else if(r>64) r=64;
    return (UBYTE)r;
}


/******************************************************************************
* ITPATTERN
******************************************************************************/


MMODULE_IT::ITPATTERN::ITPATTERN()
{
    rows=0;
    data=NULL;
    indx=NULL;
}


MMODULE_IT::ITPATTERN::~ITPATTERN()
{
    delete[] data;
    delete[] indx;
}



void MMODULE_IT::ITPATTERN::OneRow(UBYTE * &in,ITNOTE n[64],ITNOTE p[64])
/*
    Decodes one full row of IT notetupels 'n[64]' using the previous row 'p[64]' and the compressed data.
*/
{
    UBYTE chv;

    // clear row
    for(int t=0;t<64;t++) n[t].msk=0;

    // read new data for the channels that have changed
    while(chv=*in++){
        UBYTE chn=(chv-1)&63;
        n[chn].ITDecode(in,chv,p[chn]);
    }
}



int MMODULE_IT::ITPATTERN::Precode(UBYTE *in)
/*
    Do a full decode of the current IT pattern and return the number of bytes needed to convert this IT data
    into my own easy format.
*/
{
    ITNOTE n[64];
    ITNOTE p[64];
    int row,result=0;
    
    for(row=0;row<rows;row++){
        OneRow(in,n,p);
        for(int t=0;t<64;t++) result+=n[t].EZCount();
        result++;           /* increase ezcount by 1 (end-of-row sentinel) */
    }

    return result;
}



void MMODULE_IT::ITPATTERN::Recode(UBYTE *in,UBYTE *out)
{
    ITNOTE n[64];
    ITNOTE p[64];
//  UBYTE *o=out;

    for(int row=0;row<rows;row++){
        OneRow(in,n,p);
        for(int t=0;t<64;t++) n[t].EZEncode(out,t);
        *out++=0;
    }

//  return out-o;
}



UBYTE *MMODULE_IT::ITPATTERN::DecodeEZRow(UBYTE *p,ITNOTE n[64])
/*
    Decodes one EZ format row of notetuples from p into n.
    p should point to the start of the row data.

    This routine only touches those channels that actually contain data, so to get a correct result the caller
    should clear the 'msk' values of all 64 notetuples before calling this.

    Returns: 
        pointer to the next row
*/
{
    UBYTE chn;

    while(chn=*p++){
        chn--;
        n[chn].EZDecode(p);
    }

    return p;
}



void MMODULE_IT::ITPATTERN::DecodeEZRow(int row,ITNOTE n[64])
/*
    Decodes one EZ format row of this pattern into n.
    the indx array member should contain valid indexes.
*/
{
    int t;

    // clear row
    for(t=0;t<64;t++) n[t].msk=0;
    
    // check for invalid rowno?
    if(row<0 || row>rows) return;
    
    // decode the row
    DecodeEZRow(&data[indx[row]],n);
}



void MMODULE_IT::ITPATTERN::Load(MINPUT *in,long offset)
{
    if(offset>0){
        UWORD length;

        in->seek(offset,SEEK_SET);          // seek to position of pattern data
        length=in->read_UWORD();            // read length
        rows=in->read_UWORD();              // read nr. of rows
        in->seek(4,SEEK_CUR);               // skip 4 bytes

//      printf("Pattern %d bytes %d rows",length,rows);

        if(length==0 || rows<32 || rows>200){
//          static char tmp[256];
//          sprintf(tmp,"Invalid pattern: length %d, rows %d\n",length,rows);
            THROW MikModException("Invalid pattern");
//          THROW MikModException(tmp);
        }

        UBYTE *itdata=new UBYTE[length];

//      if(itdata==NULL) THROW MikModException("Alloc failed");

        in->read_UBYTES(itdata,length);     // read IT compressed pattern data

        // allocate the pattern data buffer

//      int blalen=Precode(itdata);

//      printf(" ezlen %d",blalen);

        data=new UBYTE[Precode(itdata)];

//      if(data==NULL) THROW MikModException("Alloc failed");

        // and recode the IT format data into my own format

//      printf(" fnlen %d\n",Recode(itdata,data));
        Recode(itdata,data);

        // THROW away IT format buffer

        delete[] itdata;
    }
    else{
//      printf("Empty pattern");

        rows=64;
        data=new UBYTE[64];
        for(int t=0;t<64;t++) data[t]=0;
    }

    /*
        And finally build the indx array so we can find particular rows easily
    */

    ITNOTE dummyrow[64];
    UBYTE *p=data;

    indx=new UWORD[rows];

    for(int t=0;t<rows;t++){
        indx[t]=p-data;
        p=DecodeEZRow(p,dummyrow);
        // printf("Idx %d : %d\n",t,indx[t]);
    }
//  printf("Idx %d ",indx[t-1]);
}


/******************************************************************************
* ITMIDI
******************************************************************************/

void MMODULE_IT::ITMIDI::DefaultSFX()
{
    macro[0]=0xf0;
    macro[1]=0xf0;
    macro[2]=0x00;
    macro[3]=0x101;
    macro[4]=0x100;
}


void MMODULE_IT::ITMIDI::DefaultZXX(UBYTE v)
{
    macro[0]=0xf0;
    macro[1]=0xf0;
    macro[2]=0x01;
    macro[3]=v;
    macro[4]=0x100;
}


void MMODULE_IT::ITMIDI::Load(MINPUT *in)
{
    UBYTE tmp[32],*p,c;
    UWORD *d;

    in->read_UBYTES(tmp,32);

    p=tmp;
    d=macro;

    while(1){

        c=*p; p++;

        if(c==0) break;
        
        c=tolower(c);

        if(c=='z'){
            *d++=0x101;
        }
        else if((c>='0' && c<='9') || (c>='a' && c<='f')){
        
            UBYTE hi,lo;
            
            hi=(c>='0' && c<='9') ? (c-'0') : ((c>='a' && c<='f') ? c-'a'+10 : 0);

            c=tolower(*p);
            p++;

            lo=(c>='0' && c<='9') ? (c-'0') : ((c>='a' && c<='f') ? c-'a'+10 : 0);

            *d++=(hi<<4) | lo;
        }
    }

    *d++=0x100;     // terminate midi macro
}
