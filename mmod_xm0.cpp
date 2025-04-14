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

File:           MMOD_XM0.CPP
Description:    -
Version:        1.00 - original
                1.01 - v0.91 fixed BPM setting problem
                1.02 - v1.00 envelope loops played 1 tick to long.. fixed
                1.03 - v1.00b3 a sample handle CAN be -1 (when playing an empty instrument for example)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "mmod_xm.h"

#define LIMIT(x,lo,hi) if(x<lo) x=lo; else if(x>hi) x=hi;

static UBYTE dummydata[32]={
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
};

/* linear periods to frequency translation table: */
static ULONG lintab[768] = {
        535232,534749,534266,533784,533303,532822,532341,531861,
        531381,530902,530423,529944,529466,528988,528511,528034,
        527558,527082,526607,526131,525657,525183,524709,524236,
        523763,523290,522818,522346,521875,521404,520934,520464,
        519994,519525,519057,518588,518121,517653,517186,516720,
        516253,515788,515322,514858,514393,513929,513465,513002,
        512539,512077,511615,511154,510692,510232,509771,509312,
        508852,508393,507934,507476,507018,506561,506104,505647,
        505191,504735,504280,503825,503371,502917,502463,502010,
        501557,501104,500652,500201,499749,499298,498848,498398,
        497948,497499,497050,496602,496154,495706,495259,494812,
        494366,493920,493474,493029,492585,492140,491696,491253,
        490809,490367,489924,489482,489041,488600,488159,487718,
        487278,486839,486400,485961,485522,485084,484647,484210,
        483773,483336,482900,482465,482029,481595,481160,480726,
        480292,479859,479426,478994,478562,478130,477699,477268,
        476837,476407,475977,475548,475119,474690,474262,473834,
        473407,472979,472553,472126,471701,471275,470850,470425,
        470001,469577,469153,468730,468307,467884,467462,467041,
        466619,466198,465778,465358,464938,464518,464099,463681,
        463262,462844,462427,462010,461593,461177,460760,460345,
        459930,459515,459100,458686,458272,457859,457446,457033,
        456621,456209,455797,455386,454975,454565,454155,453745,
        453336,452927,452518,452110,451702,451294,450887,450481,
        450074,449668,449262,448857,448452,448048,447644,447240,
        446836,446433,446030,445628,445226,444824,444423,444022,
        443622,443221,442821,442422,442023,441624,441226,440828,
        440430,440033,439636,439239,438843,438447,438051,437656,
        437261,436867,436473,436079,435686,435293,434900,434508,
        434116,433724,433333,432942,432551,432161,431771,431382,
        430992,430604,430215,429827,429439,429052,428665,428278,
        427892,427506,427120,426735,426350,425965,425581,425197,
        424813,424430,424047,423665,423283,422901,422519,422138,
        421757,421377,420997,420617,420237,419858,419479,419101,
        418723,418345,417968,417591,417214,416838,416462,416086,
        415711,415336,414961,414586,414212,413839,413465,413092,
        412720,412347,411975,411604,411232,410862,410491,410121,
        409751,409381,409012,408643,408274,407906,407538,407170,
        406803,406436,406069,405703,405337,404971,404606,404241,
        403876,403512,403148,402784,402421,402058,401695,401333,
        400970,400609,400247,399886,399525,399165,398805,398445,
        398086,397727,397368,397009,396651,396293,395936,395579,
        395222,394865,394509,394153,393798,393442,393087,392733,
        392378,392024,391671,391317,390964,390612,390259,389907,
        389556,389204,388853,388502,388152,387802,387452,387102,
        386753,386404,386056,385707,385359,385012,384664,384317,
        383971,383624,383278,382932,382587,382242,381897,381552,
        381208,380864,380521,380177,379834,379492,379149,378807,

        378466,378124,377783,377442,377102,376762,376422,376082,
        375743,375404,375065,374727,374389,374051,373714,373377,
        373040,372703,372367,372031,371695,371360,371025,370690,
        370356,370022,369688,369355,369021,368688,368356,368023,
        367691,367360,367028,366697,366366,366036,365706,365376,
        365046,364717,364388,364059,363731,363403,363075,362747,
        362420,362093,361766,361440,361114,360788,360463,360137,
        359813,359488,359164,358840,358516,358193,357869,357547,
        357224,356902,356580,356258,355937,355616,355295,354974,
        354654,354334,354014,353695,353376,353057,352739,352420,
        352103,351785,351468,351150,350834,350517,350201,349885,
        349569,349254,348939,348624,348310,347995,347682,347368,
        347055,346741,346429,346116,345804,345492,345180,344869,
        344558,344247,343936,343626,343316,343006,342697,342388,
        342079,341770,341462,341154,340846,340539,340231,339924,
        339618,339311,339005,338700,338394,338089,337784,337479,
        337175,336870,336566,336263,335959,335656,335354,335051,
        334749,334447,334145,333844,333542,333242,332941,332641,
        332341,332041,331741,331442,331143,330844,330546,330247,
        329950,329652,329355,329057,328761,328464,328168,327872,
        327576,327280,326985,326690,326395,326101,325807,325513,
        325219,324926,324633,324340,324047,323755,323463,323171,
        322879,322588,322297,322006,321716,321426,321136,320846,
        320557,320267,319978,319690,319401,319113,318825,318538,
        318250,317963,317676,317390,317103,316817,316532,316246,
        315961,315676,315391,315106,314822,314538,314254,313971,
        313688,313405,313122,312839,312557,312275,311994,311712,
        311431,311150,310869,310589,310309,310029,309749,309470,
        309190,308911,308633,308354,308076,307798,307521,307243,
        306966,306689,306412,306136,305860,305584,305308,305033,
        304758,304483,304208,303934,303659,303385,303112,302838,
        302565,302292,302019,301747,301475,301203,300931,300660,
        300388,300117,299847,299576,299306,299036,298766,298497,
        298227,297958,297689,297421,297153,296884,296617,296349,
        296082,295815,295548,295281,295015,294749,294483,294217,
        293952,293686,293421,293157,292892,292628,292364,292100,
        291837,291574,291311,291048,290785,290523,290261,289999,
        289737,289476,289215,288954,288693,288433,288173,287913,
        287653,287393,287134,286875,286616,286358,286099,285841,
        285583,285326,285068,284811,284554,284298,284041,283785,
        283529,283273,283017,282762,282507,282252,281998,281743,
        281489,281235,280981,280728,280475,280222,279969,279716,
        279464,279212,278960,278708,278457,278206,277955,277704,
        277453,277203,276953,276703,276453,276204,275955,275706,
        275457,275209,274960,274712,274465,274217,273970,273722,
        273476,273229,272982,272736,272490,272244,271999,271753,
        271508,271263,271018,270774,270530,270286,270042,269798,
        269555,269312,269069,268826,268583,268341,268099,267857 };


    /* Period table for Fasttracker 2 module playing: */
static unsigned gmpPeriodsFT2[96+8] = {
        907,900,894,887,881,875,868,862,856,850,844,838,832,826,820,814,
        808,802,796,791,785,779,774,768,762,757,752,746,741,736,730,725,
        720,715,709,704,699,694,689,684,678,675,670,665,660,655,651,646,
        640,636,632,628,623,619,614,610,604,601,597,592,588,584,580,575,
        570,567,563,559,555,551,547,543,538,535,532,528,524,520,516,513,
        508,505,502,498,494,491,487,484,480,477,474,470,467,463,460,457,
        453,450,447,443,440,437,434,431 };


static UBYTE avibtab[128]={
        0,1,3,4,6,7,9,10,12,14,15,17,18,20,21,23,
        24,25,27,28,30,31,32,34,35,36,38,39,40,41,42,44,
        45,46,47,48,49,50,51,52,53,54,54,55,56,57,57,58,
        59,59,60,60,61,61,62,62,62,63,63,63,63,63,63,63,
        64,63,63,63,63,63,63,63,62,62,62,61,61,60,60,59,
        59,58,57,57,56,55,54,54,53,52,51,50,49,48,47,46,
        45,44,42,41,40,39,38,36,35,34,32,31,30,28,27,25,
        24,23,21,20,18,17,15,14,12,10,9,7,6,4,3,1
};


int MMODULE_XM::GetNumChannels()
{
    return numchn;
}

int MMODULE_XM::GetNumInstruments()
{
    return numins;
}

int MMODULE_XM::GetSongLength()
{
    return songlength;
}


int MMODULE_XM::IsReady()
{
    return isready;
}


int MMODULE_XM::Test(MINPUT *i,char *title,int maxlen)
{
    UBYTE id[17];

    i->setmode(MINPUT::INTEL);
    i->rewind();
    if(!i->read_UBYTES(id,17)) return 0;

    /* xm ? */
    if(!memcmp(id,"Extended Module: ",17)) return 1;
    return 0;
}


char *MMODULE_XM::GetSongTitle()
{
    return songname;
}



char *MMODULE_XM::GetInstrumentName(int i)
{
    return instruments[i].name;
}


MMODULE_XM::MMODULE_XM() : MMODULE()
{
    /* hier members initialiseren */

    isplaying=0;
    instruments=NULL;
    patterns=NULL;
}


void MMODULE_XM::Load(MINPUT *i) 
{
    int t;

    in=i;
    
    moduletype="Fasttracker II";
                
    /* try to read module header */

    in->setmode(MINPUT::INTEL);
    in->rewind();

    in->seek(17,SEEK_CUR);              // skip id
    in->read_STRING(songname,20);       
    in->seek(1,SEEK_CUR);               // skip eof
    in->read_UBYTES(trackername,20);        

    in->read_UWORD();                   // skip version
    ULONG headersize=in->read_ULONG();          /* header size */
    songlength=in->read_UWORD();
    restart=in->read_UWORD();
    numchn=in->read_UWORD();
    numpat=in->read_UWORD();                   /* (word) Number of patterns (max 256) */
    numins=in->read_UWORD();                   /* (word) Number of instruments (max 128) */
    flags=in->read_UWORD();                    /* (word) Flags: bit 0: 0 = Amiga frequency table (see below) 1 = Linear frequency table */
    defaultspeed=in->read_UWORD();             /* (word) Default speed */
    defaultbpm=in->read_UWORD();               /* (word) Default BPM */
    in->read_UBYTES(orders,256);

    if(infomode) return;                // don't load other stuff in info mode

	if(in->eof()) THROW MikModException("Header too short");

	/* Skip extra bytes after header */

	in->seek(headersize+60-XMHEADERSIZE,SEEK_CUR);

	/* allocate pattern array */

	patterns=new XMPATTERN[numpat+1];

	// load each pattern

	for(t=0;t<numpat;t++){
		patterns[t].Load(in,numchn);
		TerminateThrow();
	}

	patterns[t].dummy();

	/* allocate instrument array */

	instruments=new XMINSTRUMENT[numins+1];

	// load each instrument

	for(t=0;t<numins;t++){
//			printf("Loading instrument %d\n",t);
		instruments[t].Load(in);
		TerminateThrow();
	}

	// plus one dummy instrument

	instruments[numins].dummy();

	/* .... */

//		printf("Songname             : %-20.20s\n",songname);
//		printf("Created with tracker : %-20.20s\n",trackername);
}


MMODULE_XM::~MMODULE_XM()
{
    Stop();
	delete[] patterns;
	patterns=NULL;
	delete[] instruments;
	instruments=NULL;
}


void MMODULE_XM::Restart()
{
	int t;

	isplaying=0;
	for(t=0;t<numchn;t++) driver->VoiceStop(audio[t].voice);

	timestamp=0;

	speed=(defaultspeed==0) ? 6 : defaultspeed;
    tick=speed;

	bpm=(UBYTE)defaultbpm;
	pattern=&patterns[0];

	breakpos=0;
	posjump=0;
	patdelayc=0;
	patdelayd=0;
	loopcnt=0;
	looppos=0;
	songpos=0;
	row=-1;
	globalvolume=64;
	globalslide=0;

	for(t=0;t<numchn;t++){
		int v=audio[t].voice;
		memset(&audio[t],0,sizeof(AUDT));
		audio[t].voice=v;
		audio[t].panning=0x80;
		audio[t].s=&dummysample1;
		audio[t].i=&instruments[numins];	// &dummyinstrument;
		audio[t].parent=this;
		audio[t].filtercutoff=0x7f;
		audio[t].filterdamping=0;
	}

	driver->SetBPM(bpm);
	isready=0;
	isplaying=1;
}


void MMODULE_XM::Start(MDRIVER *d)
{
	if(isplaying) return;

	int t;

	driver=d;

	timestamp=0;

	speed=(defaultspeed==0) ? 6 : defaultspeed;
    tick=speed;

	bpm=(UBYTE)defaultbpm;
	pattern=&patterns[0];

	breakpos=0;
	posjump=0;
	patdelayc=0;
	patdelayd=0;
	loopcnt=0;
	looppos=0;
//    forbid=0;

	songpos=0;
	row=-1;
	globalvolume=64;
	globalslide=0;

	memset(&dummysample0,0,sizeof(XMSAMPLE));
	dummysample0.volume=64;
	dummysample0.panning=0x80;
	dummysample0.handle=-1;

	memset(&dummysample1,0,sizeof(XMSAMPLE));
	dummysample1.volume=0;
	dummysample1.panning=0x80;
	dummysample1.handle=-1;

	for(t=0;t<numchn;t++){
		memset(&audio[t],0,sizeof(AUDT));
		audio[t].voice=driver->AllocVoice();
		audio[t].panning=0x80;
		audio[t].s=&dummysample1;
		audio[t].i=&instruments[numins];	// &dummyinstrument;
		audio[t].parent=this;
		audio[t].filtercutoff=0x7f;
		audio[t].filterdamping=0;
	}

	driver->ampfactor=(float)pow(driver->channels,0.52);
	driver->SetBPM(bpm);

	// connect samples
	for(t=0;t<numins;t++) instruments[t].Connect(driver);
	
	isready=0;
	isplaying=1;
}


int MMODULE_XM::GetPos()
{
	if(!isplaying) return 0;
	return songpos;
}


void MMODULE_XM::NextPos()
{
	if(!isplaying) return;
	for(int t=0;t<numchn;t++) driver->VoiceStop(audio[t].voice);
	posjump=songpos+2;
}


void MMODULE_XM::PrevPos()
{
	if(!isplaying) return;
	if(songpos){
		for(int t=0;t<numchn;t++) driver->VoiceStop(audio[t].voice);
		posjump=songpos;
	}
}


void MMODULE_XM::Stop()
{
	int t;

    if(!isplaying) return;

	// free all voices
	for(t=0;t<numchn;t++){
		driver->FreeVoice(audio[t].voice);
	}

	// disconnect samples
	for(t=0;t<numins;t++) instruments[t].Disconnect(driver);

	isplaying=0;
}


UBYTE *MMODULE_XM::FetchNIVED(UBYTE *p,UBYTE &nte,UBYTE &ins,UBYTE &vol,UBYTE &eff,UBYTE &dat)
{
	UBYTE flag=*(p++);

	nte=ins=vol=eff=dat=0;

	if(flag&0x80){
		if(flag&1)  nte=*p++;
		if(flag&2)  ins=*p++;
		if(flag&4)  vol=*p++;
		if(flag&8)  eff=*p++;
		if(flag&16) dat=*p++;
	}
	else{
		nte=flag;
		ins=*p++;
		vol=*p++;
		eff=*p++;
		dat=*p++;
	}
	return p;
}



MMODULE_XM::XMINSTRUMENT * MMODULE_XM::GetInstrument(UBYTE ins)
{
	return (ins<numins) ? &instruments[ins] : &instruments[numins];
}



MMODULE_XM::XMSAMPLE * MMODULE_XM::GetSample(XMINSTRUMENT *i,UBYTE note)
{
	UBYTE snr=i->what[note];
	return (snr<i->numsmp) ? &i->samples[snr] : &dummysample0;
}



SWORD MMODULE_XM::DoPan(SWORD envpan,SWORD pan)
{
	return(pan + (((envpan-32)*(128-abs(pan-128)))/32));
}


ULONG MMODULE_XM::getfrequency(UWORD period)
{
	ULONG result;

	if(flags & XM_LINEAR){
		result=lintab[period % 768] >> (period / 768);
	}
	else{
		result=(8363L*1712L)/period;
	}
	return result;
}


UWORD MMODULE_XM::getperiod(UBYTE note,SBYTE fine)
{
	UWORD result;
	int rnote, roct, rfine;
	int per1, per2;

	if(flags & XM_LINEAR){
		result=(10L*12*16*4)-((UWORD)note*16*4)-(fine/2);
	}
	else{
		/* Calculate period value for this note: */

		rnote = note%12;
		roct  = note/12;
		rfine = fine / 16;

		rnote = rnote << 3;
		per1  = gmpPeriodsFT2[rnote+rfine+8];

		if ( fine < 0 )
		{
			rfine--;
			fine = -fine;
		}
		else
			rfine++;

		per2 = gmpPeriodsFT2[rnote+rfine+8];

		rfine = fine & 0xF;
		per1 *= 16-rfine;
		per2 *= rfine;
		result = ((per1 + per2) * 2) >> roct;
	}
	return result;
}



UBYTE MMODULE_XM::ProcessEnvelope(UWORD *position,UBYTE keyon,UBYTE flags,UWORD points,UWORD sus,UWORD beg,UWORD end,UBYTE *table)
{
	UBYTE v;
	UWORD p=*position;

	v=table[p];

	/* sustain envelope ? */

	if((flags&XE_SUSTAIN) && keyon && p>=sus){
		/* sustain! */
		p=sus;
	}
	else{
		/* don't sustain, so increase pointer. */

        p++;

        /* looping? */

        if((flags & XE_LOOP) && p>=end){

            /* if p reached end of loop, reset p to beginning of loop */

            p=beg;
        }
        else if(p>points){

            /* if p reached the end of the envelope, stop it */

            p=points;
        }
    }

    *position=p;
    return v;
}



void MMODULE_XM::Update()
{
    int t;
    UBYTE *rowptr;
    UWORD patno;

    if(!isplaying || isready) return;

    if(speed==0){
        isready=1;
    }

    if(speed && ++tick>=speed){
        tick=0;
        row++;

        if(patdelayd){
            if(++patdelayc>patdelayd){
                patdelayd=0;
                patdelayc=0;
            }
            else row--;
        }

        /* if this was the last row, or if a patternbreak is active ->
           increase the song position */

        if(row>=pattern->numrows || breakpos || posjump){
            row=0;

            if(posjump){
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
                songpos=restart;
            }
            else if(songpos<0)
                songpos=songlength-1;
        }

        /* get a pointer to the current pattern */

        patno=orders[songpos];
        pattern=(patno < numpat) ? &patterns[patno] : &patterns[numpat];

        if(breakpos){
            breakpos--;
            if(breakpos<pattern->numrows) row=breakpos;
            breakpos=0;
        }
    }

//  printf("row : %d\n",speed,bpm,row);

    /* reset the pointer to the current pattern row */

    if(pattern->data==NULL)
        rowptr=dummydata;
    else
        rowptr=pattern->data+pattern->index[row];

    for(t=0;t<numchn;t++){
        UBYTE nte,ins,vol,eff,dat;
        rowptr=FetchNIVED(rowptr,nte,ins,vol,eff,dat);
        audio[t].HandleTick(nte,ins,vol,eff,dat);
    }

    /* process the envelopes & start the samples */

    if(!scanmode) for(t=0;t<numchn;t++){

        UBYTE envvol,envpan;
        UWORD period;
        ULONG tmp;

        AUDT *a=&audio[t];
        XMINSTRUMENT *i=a->i;
        XMSAMPLE *s=a->s;

        if(a->kick){
            if(s->handle>=0){       // fix 1.00b3 : a sample handle CAN be -1 (when playing an empty instrument for example)

                UWORD flag=0;

                ULONG start,
                      length=s->length,
                      loops=s->loopstart,
                      loope=s->loopend;

                start=(a->altstart) ? a->startpos : 0;

                if( (s->mmFlg&SF_LOOP && (start<loope)) ||
                    (start<length) ){

                    driver->VoicePlay(a->voice,
                                    s->handle,
                                    start,
                                    length,
                                    loops,
                                    loope,
                                    0,0,
                                    s->mmFlg|SF_FTPAN);
                }
            }
            else{
                driver->VoiceStop(a->voice);
            }
            a->kick=0;
        }

        /* set default envelope values */

        envvol=64;
        envpan=32;

        /* do we have to process the volume envelope ? */

        if(i->volenv.flag & XE_ON){
            envvol=ProcessEnvelope(&a->venvpos,
                                    a->keyon,
                                    i->volenv.flag,
                                    i->volenv.points-1,
                                    i->volenv.sus,
                                    i->volenv.beg,
                                    i->volenv.end,
                                    i->volenv.envelope);
        }

        /* do we have to process the panning envelope ? */

        if(i->panenv.flag & XE_ON){
            envpan=ProcessEnvelope(&a->penvpos,
                                    a->keyon,
                                    i->panenv.flag,
                                    i->panenv.points-1,
                                    i->panenv.sus,
                                    i->panenv.beg,
                                    i->panenv.end,
                                    i->panenv.envelope);
        }

        /* do we have to process the autovibrato ? */

        /*
            Use realperiod as base period, because autovibrato is ADDED
            to any normal vibrato.
        */

        period=a->realperiod;


        if(period && i->vibdepth){
            SWORD vibval,vibdpt;

            switch(i->vibflg){

                case 0:
                    vibval=avibtab[a->avibpos&127];
                    if(a->avibpos&0x80) vibval=-vibval;
                    break;

                case 1:
                    vibval=64;
                    if(a->avibpos&0x80) vibval=-vibval;
                    break;

                case 2:
                    /* avibpos&255      : stijgende lijn [0-255] */
                    /* (avibpos&255>>1) : stijgende lijn [0-127] */
                    /* 63-(avibpos&255>>1) : dalende lijn [63- -64] */
                    /* 63-((avibpos+128)&255>>1) : dalende lijn [63- -64] */
                    vibval=63-(((a->avibpos+128)&255)>>1);
                    break;

                case 3:
                    vibval=(((a->avibpos+128)&255)>>1)-64;
                    break;
            }

            if(a->keyon){
                if(a->aswppos < i->vibsweep){
                    vibdpt=(a->aswppos*i->vibdepth*4)/i->vibsweep;
                    a->aswppos++;
                }
                else{
                    vibdpt=i->vibdepth<<2;
                }
            }
            else{
                /*
                    key-off -> depth becomes 0 if final depth wasn't reached
					or stays at final level if depth WAS reached
				*/

				if(a->aswppos>=i->vibsweep)
					vibdpt=i->vibdepth<<2;
				else
					vibdpt=0;
			}

			vibval=(vibval*vibdpt)>>8;
			period-=vibval;

			LIMIT(period,40,50000);

			/* update vibrato position */

			a->avibpos=(a->avibpos+i->vibrate)&0xff;
		}

		tmp=a->fadevol;		/* max 32768 */
		tmp*=globalvolume;	/* max 64 */
		tmp>>=6;			/* =max 32768 */

        tmp*=envvol;		/* * max 64 */
		tmp*=a->realvolume;	/* * max 64 */
        tmp/=32768L*16;
        if(tmp>255) tmp=255;

		driver->VoiceSetVolume(a->voice,(UBYTE)tmp);
		driver->VoiceSetPanning(a->voice,(UBYTE)DoPan(envpan,a->panning));
		if(period) driver->VoiceSetFrequency(a->voice,getfrequency(period));
		driver->VoiceSetFilter(a->voice,a->filtercutoff,a->filterdamping);

		/*  if key-off, start substracting
			fadeoutspeed from fadevol: */

		if(!a->keyon){
			if(i->volenv.flag & XE_ON){
				if(a->fadevol>=i->volfade)
					a->fadevol-=i->volfade;
                else
					a->fadevol=0;
            }
            else a->SetVolume(0);
		}

		/* if period-dropback flag is set, restore realperiod to period */

		if(a->perdropback){
			a->realperiod=a->period;
			a->perdropback=0;
		}
	}

	driver->SetBPM(bpm);			// <- v0.91 fixed BPM problem
	timestamp+=(125L*1000)/(50L*bpm);
}


int MMODULE_XM::GetTimeStamp()
{
	return timestamp;
}


void MMODULE_XM::SetTimeStamp(int ts)
{
	timestamp=ts;
}


int MMODULE_XM::GetRow()
{
  return row;
}
