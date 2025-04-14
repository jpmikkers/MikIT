/*

File:           MMOD_IT0.CPP
Description:    -
Version:        1.00 - original
                1.01 - new info retrieving functions, high offset SAx back in
                1.02 - fixed hanging notes on prevpos and nextpos
                1.03 - fixed bug where effect SB0 would stop a song if loopmode was off
                1.04 - fixed bug: don't load IT patterns that aren't contained in the Order
                1.05 - v0.91 implemented surround effect S91
                1.06 - v0.91 fixed Gxx to fix the disappearing bass in meskitbox.it
                1.07 - v0.99 implemented resonant filtering
                1.08 - v0.99 Oxx beyond the end of a sample should start from the end when old effects are enabled (Kohan Ikin)
                1.09 - v0.99 support surround on default channel pannings (odingalt@onyx.digisys.net)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mmod_it.h"

// #define POORMANDEBUG

#ifdef POORMANDEBUG
#define POORDEBUG(x) puts(x);
#else
#define POORDEBUG(x)
#endif


int MMODULE_IT::GetNumChannels()
{
    return 128;
}

int MMODULE_IT::GetNumSamples()
{
    return SmpNum;
}

int MMODULE_IT::GetNumInstruments()
{
    return InsNum;
}

char *MMODULE_IT::GetSongTitle()
{
    return songname;
}

char *MMODULE_IT::GetSampleName(int s)
{
    return Samples[s].smpname;
}

char *MMODULE_IT::GetInstrumentName(int i)
{
    return Instruments[i].insname;
}


int MMODULE_IT::GetSongMessage(char *d,int l)
{
    if(l==0){
        return (Message) ? MsgLgth : 0;
    }
    else{
        if(Message) strncpy(d,Message,l); 
        else d[0]=0;
    }
    return MsgLgth;
}


void MMODULE_IT::LoadHeader()
{
        UBYTE id[4];

        moduletype="Impulse Tracker";
                
        /* try to read module header */

        in->setmode(MINPUT::INTEL);
        in->rewind();

        in->read_UBYTES(id,4);

        if(!memcmp(id,"IMPI",4)) THROW MikModException("Old impulse formats are not supported");
        if(!memcmp(id,"ziRC",4)) THROW MikModException("Can't load compressed modules");
                
        in->read_STRING(songname,26);
        in->seek(2,SEEK_CUR);            // skip xx
                
        OrdNum=in->read_UWORD();
        InsNum=in->read_UWORD();
        SmpNum=in->read_UWORD();
        PatNum=in->read_UWORD();
        Cwtv=in->read_UWORD();
        Cmwt=in->read_UWORD();

//      if(Cmwt<0x200) THROW MikModException("Can't load old instrument format (yet) - Use Impulse tracker to convert to modern format");
        
        Flags=in->read_UWORD();
        Special=in->read_UWORD();

        GV=in->read_UBYTE();
        MV=in->read_UBYTE();
        IS=in->read_UBYTE();
        IT=in->read_UBYTE();
        Sep=in->read_UBYTE();
        in->seek(1,SEEK_CUR);            // skip (0)

        MsgLgth=in->read_UWORD();
        MsgOffs=in->read_ULONG();
        in->seek(4,SEEK_CUR);            // skip xxxx

        in->read_UBYTES(chnpan,64);
        in->read_UBYTES(chnvol,64);

        if(OrdNum){
                int count,q;

                POORDEBUG("Loading orders");
                
                // allocate and load orders array
                orders=new UBYTE[OrdNum];
                in->read_UBYTES(orders,OrdNum);

                for(count=q=0;q<OrdNum;q++){
                    if(orders[q]<0xfe) count++;
                }

                if(count==0) THROW MikModException("Nothing to play, no orders");
        }                        

        if(InsNum){
                POORDEBUG("Loading Instrument offsets");

                // allocate and load instrument offsets
                insoffsets=new ULONG[InsNum];
                in->read_ULONGS(insoffsets,InsNum);
        }                        

        if(SmpNum){
                POORDEBUG("Loading sample offsets");

                // allocate and load sample offsets
                smpoffsets=new ULONG[SmpNum];
                in->read_ULONGS(smpoffsets,SmpNum);
        }
                
        if(PatNum){
                POORDEBUG("Loading pattern offsets");

                // allocate and load pattern offsets
                patoffsets=new ULONG[PatNum];
                in->read_ULONGS(patoffsets,PatNum);
        }                       

        if(in->eof()) THROW MikModException("Header too short");

        if(Special & 2){
            // session information is present
            UWORD numelem=in->read_UWORD();
            in->seek(8L*numelem,SEEK_CUR);      // skip session information
        }

        if(Special & 8){ 
            // midi macros are present
            int q;
            for(q=0;q<9  ;q++) midiini[q].Load(in);
            for(q=0;q<16 ;q++) midisfx[q].Load(in);
            for(q=0;q<128;q++) midizxx[q].Load(in);
        }                       
        else{
            // midi macros not present, use defaults
            int q;
            for(q=0;q<16 ;q++) midisfx[q].DefaultSFX();
            for(q=0;q<128;q++) midizxx[q].DefaultZXX(q);
        }

        if(Special & 1){
                POORDEBUG("Loading message");
                in->seek(MsgOffs,SEEK_SET);   // skip to message
                Message=new char[MsgLgth+1];
                in->read_STRING(Message,MsgLgth);
        }                       

        /* .... */
        
//        printf("Songname             : %s\n",songname);

//        printf("Created with tracker : %d.%02x\n",Cwtv>>8,Cwtv&0xff);
//        printf("Compatible with      : %d.%02x\n",Cmwt>>8,Cmwt&0xff);
//        printf("Flags                : 0x%04x\n",Flags);
//        printf("Special              : 0x%04x\n",Special);
//        printf("Global volume(0->128): %d\n",GV);
//        printf("Mix volume (0->128)  : %d\n",MV);
//        printf("Initial Speed of song: %d\n",IS);
//        printf("Initial Tempo of song: %d\n",IT);
//        printf("Panning separation   : %d\n",Sep);
//        printf("Chnl Vol : ");
//        for(t=0;t<64;t++) printf("%02d ",chnvol[t]);
//        printf("\nChnl Pan : ");
//        for(int t=0;t<64;t++) printf("%02d ",chnpan[t]);                

//        printf("Ordnum               : %d\n",OrdNum);
//        printf("Insnum               : %d\n",InsNum);
//        printf("Smpnum               : %d\n",SmpNum);
//        printf("Patnum               : %d\n",PatNum);

/*        if(Message){
                for(t=0;t<MsgLgth;t++) putchar(Message[t]=='\r' ? '\n' : Message[t]);
        }
        puts("");
*/
}



MMODULE_IT::MMODULE_IT() : MMODULE()
{
    isplaying=0;
    orders=NULL;
    insoffsets=NULL;
    smpoffsets=NULL;
    patoffsets=NULL;
    Message=NULL;
    Instruments=NULL;
    Samples=NULL;
    Patterns=NULL;
}



void MMODULE_IT::Load(MINPUT *i) 
{
    int t;
    
    in=i;

    LoadHeader();

    if(infomode) return;        // don't load other stuff in info mode

    if(InsNum){
		POORDEBUG("Loading instruments");

        // allocate instrument array

        Instruments=new ITINSTRUMENT[InsNum];

        // load each instrument

        for(t=0;t<InsNum;t++){
            in->seek(insoffsets[t],SEEK_SET);            // skip xxxx
   			if(Cmwt<0x200)
				Instruments[t].LoadOld(in);
			else
				Instruments[t].Load(in);
			TerminateThrow();
        }
    }                        

    if(SmpNum){
		POORDEBUG("Loading samples");

        // allocate instrument array

        Samples=new ITSAMPLE[SmpNum];

        // load each instrument

        for(t=0;t<SmpNum;t++){
            in->seek(smpoffsets[t],SEEK_SET);            // skip xxxx
            Samples[t].Load(in);
			TerminateThrow();
        }
    }                        

    if(PatNum){

		/* IT bugfix : don't try to load patterns that 
           don't appear in the order list */

		// so visit all patterns
		
		for(t=0;t<PatNum;t++){
			int u;

			// see if this pattern 't' is in the order list

			for(u=0;u<OrdNum;u++){
				if(orders[u]==t) break;
			}

			// if not, don't load it (setting patoffset to zero will do that)

            if(u==OrdNum){
                patoffsets[t]=0;                    
            }
        }
        
        POORDEBUG("Loading patterns");

        // allocate pattern array

        Patterns=new ITPATTERN[PatNum+1];

        // load each pattern

        for(t=0;t<PatNum;t++){
//              printf("Loading pattern %d at %d\n",t,patoffsets[t]);
            Patterns[t].Load(in,patoffsets[t]);
            TerminateThrow();
        }

        // plus one dummy pattern
        Patterns[PatNum].Load(in,0);
        POORDEBUG("Patterns done");
    }                        

    if(!(Flags&4)){
        POORDEBUG("Sample -> Instrument conversion");

        /* sample mode, so translate samples to instruments first */
        delete[] Instruments;
        Instruments=new ITINSTRUMENT[SmpNum];
        InsNum=SmpNum;

        // create each instrument

        for(t=0;t<SmpNum;t++){
            Instruments[t].CreateFromSample(t+1);
            TerminateThrow();
        }
    }
}



MMODULE_IT::~MMODULE_IT()
{
    Stop();
    POORDEBUG("freeing orders");
    delete[] orders;
    orders=NULL;
    POORDEBUG("freeing insoffsets");
    delete[] insoffsets;
    insoffsets=NULL;
    POORDEBUG("freeing smpoffsets");
    delete[] smpoffsets;
    smpoffsets=NULL;
    POORDEBUG("freeing patoffsets");
    delete[] patoffsets;
    patoffsets=NULL;
    POORDEBUG("freeing message");
    delete[] Message;
    Message=NULL;
    POORDEBUG("freeing samples");
    delete[] Samples;
    Samples=NULL;
    POORDEBUG("freeing instruments");
    delete[] Instruments;
    Instruments=NULL;
    POORDEBUG("freeing patterns");
    delete[] Patterns;
    Patterns=NULL;
    POORDEBUG("done cleanup();");
}


int MMODULE_IT::Test(MINPUT *i,char *title,int maxlen)
{
    UBYTE id[4];

    i->setmode(MINPUT::INTEL);
    i->rewind();
    if(!i->read_UBYTES(id,4)) return 0;

    /* it ? */

    if(!memcmp(id,"IMPM",4)) return 1;
    if(!memcmp(id,"IMPI",4)) return 1;
    if(!memcmp(id,"ziRC",4)) return 1;
    return 0;
}



void MMODULE_IT::Restart()
{
    int t;

    isplaying=0;

    for(t=0;t<64;t++){
        if(virt[t].voice>=0) driver->VoiceStop(virt[t].voice);
    }

    for(t=0;t<64;t++){
        virt[t].owner=-1;
        virt[t].active=0;
        virt[t].background=1;
        virt[t].kick=0;
        virt[t].keyon=0;
        virt[t].cut=0;

        memset(&cols[t],0,sizeof(ITCOLUMN));
        cols[t].SetParent(this);        
        cols[t].filtercutoff=0x7f;
        cols[t].CV=chnvol[t];
        cols[t].CP=(chnpan[t]==160 || chnpan[t]==100) ? 32 : chnpan[t];     // v0.99 : support surround on channel pannings
        cols[t].NP=(chnpan[t]==160 || chnpan[t]==100) ? 32 : chnpan[t];     // v0.99 : support surround on channel pannings
        cols[t].surround=(chnpan[t]==100);                                  // v0.99 : support surround on channel pannings
    }

    // reset tick counter
    timestamp=0;
    tick=speed=IS;
    extraticks=0;
    row=-1;
    patterndelay=0;
    patterndelrq=0;
    songpos=0;
    orderjump=0;
    breaktorow=0;
    patno=orders[0]; if(patno>PatNum) patno=(UBYTE)PatNum;
    pattern=&Patterns[patno];

    // reset running global volume
    runGV=GV;   
    tempo=IT;
    driver->SetBPM(tempo);

    isplaying=1;
}



void MMODULE_IT::Start(MDRIVER *d)
{
    int t;

    if(isplaying) return;

    driver=d;

    // allocate as many voices as possible
    for(t=0;t<64;t++){
        memset(&virt[t],0,sizeof(ITVIRTCH));
        virt[t].SetParent(this);        
        virt[t].owner=-1;
        virt[t].background=1;
        virt[t].voice=driver->AllocVoice();         // (chnpan[t]==160) ? -1 : 
        virt[t].filtercutoff=0x7f;
        
        memset(&cols[t],0,sizeof(ITCOLUMN));
        cols[t].SetParent(this);        
        cols[t].filtercutoff=0x7f;
        cols[t].CV=chnvol[t];
        cols[t].CP=(chnpan[t]==160 || chnpan[t]==100) ? 32 : chnpan[t];     // v0.99 : support surround on channel pannings
        cols[t].NP=(chnpan[t]==160 || chnpan[t]==100) ? 32 : chnpan[t];     // v0.99 : support surround on channel pannings
        cols[t].surround=(chnpan[t]==100);                                  // v0.99 : support surround on channel pannings
    }

    // reset tick counter
    timestamp=0;
    tick=speed=IS;
    extraticks=0;
    row=-1;
    patterndelay=0;
    patterndelrq=0;
    songpos=0;
    orderjump=0;
    breaktorow=0;
    patno=orders[0]; if(patno>PatNum) patno=(UBYTE)PatNum;
    pattern=&Patterns[patno];

    // reset running global volume
    runGV=GV;   

    // set mixing volume multiplier.. the IT multiplier is based on 128 channels so compensate for the number
    // we are actually using

    driver->ampfactor=((float)driver->channels*MV)/128.0f;

//  printf("MV %d, channels %d, ampfactor %f\n",MV,driver->channels,driver->ampfactor);

    tempo=IT;
    driver->SetBPM(tempo);

    // connect samples to the device
    
    for(t=0;t<SmpNum;t++){
        Samples[t].handle=driver->PrepareSample(Samples[t].msample);
    }

    isready=0;
    isplaying=1;
}


int MMODULE_IT::IsReady()
{
    return isready;
}


int MMODULE_IT::GetPos()
{
    if(!isplaying) return 0;
    return songpos;
}


int MMODULE_IT::GetSongLength()
{
    return OrdNum;
}


void MMODULE_IT::NextPos()
{
    if(!isplaying) return;
    for(int t=0;t<64;t++){
        if(virt[t].voice>=0) driver->VoiceStop(virt[t].voice);
    }
    orderjump=songpos+2;
}


void MMODULE_IT::PrevPos()
{
    if(!isplaying) return;
    for(int t=0;t<64;t++){
        if(virt[t].voice>=0) driver->VoiceStop(virt[t].voice);
    }
    orderjump=songpos;
}


void MMODULE_IT::Stop()
{
    int t;

    if(!isplaying) return;

    // free all voices
    for(t=0;t<64;t++) driver->FreeVoice(virt[t].voice);

    // disconnect samples
    for(t=0;t<SmpNum;t++){
        driver->UnPrepareSample(Samples[t].handle);
        Samples[t].handle=-1;
    }

    isplaying=0;
}



void MMODULE_IT::ITCOLUMN::FindSmpNoteFreq()
/*
    Finds the new sample, not and frequency if a new note or new instrument was
*/
{
    if(ins>0 && ins<=m->InsNum){                            // 0.02B FIX : ins<=m->InsNum prevent illegal instrument numbers
        ITINSTRUMENT *i=&m->Instruments[ins-1];

        smp     =i->notesample[nte].sample;
        smpnote =i->notesample[nte].note;
        frq=PitchTable[smpnote];

        if(smp && smp<=m->SmpNum){
            frq=MulDiv(frq,m->Samples[smp-1].C5Speed,65536);
        }

        /* set panning to default panning */
        
        NP=CP;

        /* use sample default panning if enabled */

        /* a default sample pan overrides instrument default pan and instrument pan separation */
        
        if(smp && (m->Samples[smp-1].DfP & 128)) NP=m->Samples[smp-1].DfP&127;
        else{
            if(!(i->DfP & 128)) NP=i->DfP&127;

            SWORD t=NP;

            t=t+((((SWORD)nte-(SWORD)i->PPC) * i->PPS) / 8);
            if(t<0) t=0; else if(t>64) t=64;

            NP=(UBYTE)t;
        }

        /* and apply the random panning, if needed */
        NP=i->RandomPan(NP);

        /* set default filter cutoff and damping */
        if(i->defaultcutoff & 0x80) filtercutoff=i->defaultcutoff&0x7f;
        if(i->defaultdamping & 0x80) filterdamping=i->defaultdamping&0x7f;
    }
}


void MMODULE_IT::ITCOLUMN::ProcessTick0()
{
    if((n.msk & 8) && n.eff==19 && (n.dat>>4)==0xd){
        if(tick!=(n.dat&0xf)) return;
    }
    else{
        if(tick!=0) return;
    }
    
    int doingslide=0;

    if((n.msk & 2) && (n.ins>m->InsNum)){
        n.msk&=~2;
        n.msk&=~1;
    }

    /* do some note portamento preprocessing */

    if( (   ((n.msk&4) && (n.vol>=193 && n.vol<=202)) ||        //
            ((n.msk&8) && (n.eff==7 || n.eff==12)) ) &&         // noteportamento effect ?
        ((n.msk&1) && n.nte<=119) ){        // + a note

        doingslide=1;
        
        UBYTE slideins=ins;
        if(n.msk&2) slideins=n.ins;

        if(slideins>0){
            ITINSTRUMENT *slidei=&m->Instruments[slideins-1];

            UBYTE smp=slidei->notesample[n.nte].sample;
            UBYTE smpnote=slidei->notesample[n.nte].note;

            if(smp && smp<=m->SmpNum){
                destfrq=PitchTable[smpnote];
                destfrq=MulDiv(destfrq,m->Samples[smp-1].C5Speed,65536);
            }
            else{
                destfrq=0;                  // no good dest frequency note, so don't do slide
				n.dat=0;
			}
		}

        // GXX bug fix april 1997

		if(!v || !v->active) doingslide=0;	// v0.91: added the || !v->active to fix the disappearing bass in meskitbox.it
	}
	
	// new note? set it and start sample
	
	if(n.msk & 1){
		if(n.nte==255){
			/* note off */
			if(v) v->keyon=0;
		}
		else if(n.nte==254){
			/* note cut */
			if(v){
				v->cut=1;
				n.msk&=~2;		// 0.04B FIX : can't be any instrument when note cut is active
            }
        }
        else if(nte<=119){
            if(!doingslide){
                nte=n.nte;
                kick=1;
                FindSmpNoteFreq();
            }
        }
    }

    // new instrument? set it and start it

    if((n.msk & 2) && (n.ins<=m->InsNum)){
        ins=n.ins;

        if(!doingslide && (n.msk&1)){           // only restart if no slide, and (0.05) note present
            kick=1;

            /* work out what sample, what note and what frequency belongs to 
               this note */

            FindSmpNoteFreq();
            tremorptr=0;
            vibptr=0;
            trmptr=0;
            qptr=0;
        }
    
        /* set volume to default sample volume */

        if(smp && smp<=m->SmpNum) vol=m->Samples[smp-1].Vol;
    }

    // new volume

    if(n.msk & 4){
        if(n.vol<=64){
            vol=n.vol;
        }
        else if(n.vol>=128 && n.vol<=192){
            CP=n.vol-128;   
            NP=CP;
        }
    }

    // invalid samples should not be played

    if(ins==0 || smp==0 || ins>m->InsNum || smp>m->SmpNum) kick=0;      // 0.02B Fix : prevent illegal samples/instruments
}


MMODULE_IT::ITVIRTCH *MMODULE_IT::vFindLeastActive()
{
    int t;

    // first try finding a slave channel that isn't doing anything
	// or find a background slave at the lowest volume

	ITVIRTCH *result=NULL;
	UWORD minNFC=0xffff;

	for(t=0;t<64;t++){
		ITVIRTCH *v=&virt[t];

		if(v->voice>=0){
			if(!v->active) return v;		// <- bumped into an unused channel, so use that

			if(v->background && v->NFC<minNFC){
				minNFC=v->NFC;
				result=v;
			}
		}
	}

	return result;
}



void MMODULE_IT::ITVIRTCH::SetParent(MMODULE_IT *im)
{
	m=im;
}


void MMODULE_IT::ITCOLUMN::SetParent(MMODULE_IT *im)
{
	m=im;
}


void MMODULE_IT::ITVIRTCH::StartVolEnv()
{
	if(i) volprc.Start(&(i->volenv));
}


void MMODULE_IT::ITVIRTCH::StartPanEnv()
{
	if(i) panprc.Start(&(i->panenv));
}


void MMODULE_IT::ITVIRTCH::StartPtcEnv()
{
	if(i) ptcprc.Start(&(i->ptcenv));
}


void MMODULE_IT::ITVIRTCH::Update()
{
	MDRIVER *driver=m->driver;

	if(cut){
		cut=0;
		driver->VoiceStop(voice);
		active=0;
	}

	if(kick && i && s){
		// voice is started
		// reset NoteFadeComponent 

		NFC=1024;

		// set sample and instrument volume

		SV=s->GvL;
		IV=i->GetGbV();		// get instrument volume (with applied random factor)
		NNA=i->NNA;

		// reset vibrato counters

		vibrunp=0;
		vibrund=0;
		
		if(startoffset>=s->Length){				// start offset exceeds sample size
			if(m->Flags&16){						// old effects?
				startoffset=s->Length-1;		// yup, start playing from last sample
			}
			else{
				startoffset=0;					// no, ignore Oxx command 
			}
		}

		driver->VoiceStop(voice);

		driver->VoicePlay(voice,
				s->handle,
				startoffset,
				s->Length,
				s->LoopBeg,s->LoopEnd,
				s->SLoopBeg,s->SLoopEnd,
				s->mmFlg | (surround ? SF_SURROUND : 0));

		kick=0;
	}

	
	/* calculate and set the volume */
	
	UWORD envtick=volprc.GetTick();
	VEV=(envtick==0xffff) ? 64 : i->volenv.values[envtick];
	ULONG endvol;
	endvol=m->runGV;	// 128	
	endvol*=SV;			// 64
	endvol*=IV;			// 128
	endvol*=CV;			// 64		CV
	endvol>>=17;		// => 512
	endvol*=vol;		// 64
	endvol*=VEV;		// 64
	endvol*=NFC;		// 1024
	endvol/=8421504;	// => 255
	driver->VoiceSetVolume(voice,(UBYTE)endvol);

	/* calculate and set panning */

	UWORD pantick=panprc.GetTick();
	SBYTE PEV=(pantick==0xffff) ? 0 : i->panenv.values[pantick];
	SWORD endpan;
	endpan=(32-abs(32-NP));
	endpan*=PEV;
	endpan>>=5;
	endpan+=NP;
	endpan*=4;
	if(endpan<0) endpan=0;
	else if(endpan>255) endpan=255;
	driver->VoiceSetPanning(voice,(UBYTE)endpan);

	/* calculate and set filter */

	/* set filter values */

	driver->VoiceSetFilter(voice,filtercutoff,filterdamping);

	if(ptcprc.Flg & 0x80){
		UWORD ptctick=ptcprc.GetTick();
		SWORD PTV=(ptctick==0xffff) ? 0 : ((i->ptcenv.values[ptctick]>>2)+64);
		/* its a filter envelope */
		driver->VoiceSetFilter(voice,(PTV*filtercutoff)/127,filterdamping); 
	}

	/* calculate and set pitch */

	if(s){

		// perform instrument pitch envelope

		UWORD ptctick=ptcprc.GetTick();
		SWORD PTV=(ptctick==0xffff) ? 0 : i->ptcenv.values[ptctick];
		SLONG endfrq=frq;			// MulDiv(frq,s->C5Speed,65536);

		if(!(ptcprc.Flg & 0x80)){
			/* its a pitch envelope */
			if(PTV<0){
				endfrq=MulDiv(endfrq,LinearSlideDownTable[-PTV],65536);
			}
			else if(PTV>0){
				endfrq=MulDiv(endfrq,LinearSlideUpTable[PTV],65536);
			}
		}

		// perform sample vibrato

		if(s->ViD){
			vibrund+=s->ViR;
			
			int val,depth=vibrund>>8;

			if(depth>s->ViD){
				depth=s->ViD;
				vibrund-=s->ViR;
			}

			if(s->ViS){
				// update pointer
				vibrunp+=s->ViS;

				if(s->ViT<3){
					val=m->VibTables[s->ViT][vibrunp];
				}
				else{
					val=(rand()&127)-64;
				}
		
				val*=depth;
				val>>=8;

				if(val<0){
					endfrq=MulDiv(endfrq,LinearSlideDownTable[-val],65536);
				}
				else if(val>0){
					endfrq=MulDiv(endfrq,LinearSlideUpTable[val],65536);
				}
			}
		}

		driver->VoiceSetFrequency(voice,endfrq);

		// do key on / key off

		if(keyon){
			driver->VoiceKeyOn(voice);
		}
		else{
			driver->VoiceKeyOff(voice);
		}
	}
	
	// update envelope pointers

	volprc.Process(keyon);
	panprc.Process(keyon);
	ptcprc.Process(keyon);

	// check for key-off fades

	if(!keyon){
		// key-off WITHOUT volume envelope -> fade note
		// OR keyoff WITH _normal_ volume envelope loop
		if(!(volprc.Flg&1) || volprc.Flg&2) dofade=1;
	}

	// volume envelope reached the end? start fading NoteFadeComponent

	if(volprc.Done()) dofade=1;
		
	if(dofade && i){
		if(NFC>=i->FadeOut) NFC-=i->FadeOut; else NFC=0;
	}
}



void MMODULE_IT::ITVIRTCH::Touch()
{
	if(active){

		// NOT playing anything when NFC, SV or IV is zero

		if(NFC==0 || SV==0 || IV==0){
			active=0;
			return;
		}

		// voice is in background and volume is zero
		
		if(background && vol==0){
			active=0;
			return;
		}

		// volume envelope active?

		if(volprc.Flg&1){
			// envelope pointer reached the end and envelope value is zero -> not playing anything
			if(VEV==0 && volprc.Done()){
				active=0;
				return;
			}
		}

		active=m->driver->VoiceActive(voice);
	}
}



void MMODULE_IT::ITVIRTCH::Disown()
/*
	Puts a slave in the background, if it wasn't already. Also takes care of detaching the
    slave from the master so the master doesn't touch the slave anymore.
*/
{
	/* check if this voice is in the foreground */

	if(!background){
		// yes, so move it to the background
		if(owner>=0) m->cols[owner].v=NULL;
		background=1;
	}
}



void MMODULE_IT::ITVIRTCH::DoNNA(UBYTE NNA)
{
	Disown();

	switch(NNA){
	
		case 0:
			// NNA is cut
			cut=1;
			active=0;
			break;

		case 1:
			// NNA is continue
			break;

		case 2:
			// NNA is note off
			keyon=0;					
			break;

		case 3:
			// NNA is note fade
			dofade=1;
			break;
	}
}



void MMODULE_IT::ITVIRTCH::DoNNA()
{
	DoNNA(NNA);
}



void MMODULE_IT::ITVIRTCH::PastNoteAction(int column,UBYTE action)
{
	if(!active) return;					// slave isn't active, no need to check
    if(!background) return;             // not in background
    if(owner!=column) return;           // owner is wrong

    switch(action){                     // what action ?
        case 0:
            DoNNA(0);                   // cut
            break;

        case 1:
            DoNNA(2);                   // note off
            break;

        case 2:
            DoNNA(3);                   // note fade
            break;
    }
}



void MMODULE_IT::ITVIRTCH::DuplicateCheck(int column,ITCOLUMN *c)
{
    UBYTE DCT=m->Instruments[c->ins-1].DCT;

    if(!DCT) return;                    // no duplicate check type

    if(!active) return;                 // slave isn't active, no need to check
	if(owner!=column) return;			// owner is wrong
	if(ins!=c->ins) return;				// not the same instrument

	if(DCT==1 && c->nte!=nte) return;	// wrong note
	if(DCT==2 && c->smp!=smp) return;	// wrong sample

	UBYTE DCA=m->Instruments[c->ins-1].DCA;

	switch(DCA){						// what action ?
		case 0:
			DoNNA(0);					// cut
			break;

		case 1:
			DoNNA(2);					// note off
			break;

		case 2:
			DoNNA(3);					// note fade
			break;
	}
}







void MMODULE_IT::ITCOLUMN::Effects()
{
	if(n.msk&4){
		UBYTE veff=0;

		if(n.vol>=65 && n.vol<=124){
			veff=1+((n.vol-65) / 10);
		}
		else if(n.vol>=193 && n.vol<=212){
			veff=7+((n.vol-193) / 10);
		}

		switch(veff){
			case 1:	VEffectA(n.vol-65);   break;
			case 2:	VEffectB(n.vol-75);   break;
			case 3:	VEffectC(n.vol-85);   break;
			case 4:	VEffectD(n.vol-95);   break;
			case 5:	VEffectE(n.vol-105);  break;
			case 6:	VEffectF(n.vol-115);  break;
			case 7:	VEffectG(n.vol-193);  break;
			case 8:	VEffectH(n.vol-203);  break;
		}
	}
	
	if(!(n.msk&8)) return;

	switch(n.eff+'A'-1){
		case 'A':			// Axx set speed
			EffectA();
			break;

		case 'B':			// Bxx Jump to order
			EffectB();
			break;

		case 'C':			// Cxx Break to row
			EffectC();
			break;

		case 'D':			// Dxx volume slide
			EffectD();
			break;

		case 'E':			// Exx pitch slide down
			EffectE();
			break;

		case 'F':			// Fxx pitch slide up
			EffectF();
			break;

		case 'G':			// Gxx note portamento
			EffectG(n.dat);
			break;

		case 'H':			// Hxx vibrato
			EffectH();
			break;

		case 'I':			// Ixx tremor
			EffectI();
			break;

		case 'J':			// Jxx arpeggio
			EffectJ();
			break;

		case 'K':			// Kxx dual command : H00 + Dxx
			DoVibrato();
			EffectD();
			break;

		case 'L':			// Lxx dual command : G00 + Dxx
			EffectG(0);
			EffectD();
			break;

		case 'M':			// M set channel volume
			EffectM();
			break;

		case 'N':			// N slide channel volume
			EffectN();
			break;

		case 'O':			// O set sample offset
			EffectO();
			break;

		case 'P':			// P panning slide
			EffectP();
			break;

		case 'Q':			// Q retrig with volume modifier
			EffectQ();
			break;

		case 'R':			// R tremolo
			EffectR();
			break;

		case 'S':			// S special
			EffectSx();
			break;

		case 'T':			// T set/slide tempo
			EffectT();
			break;

		case 'U':			// U fine vibrato
			EffectU();
			break;

		case 'V':			// V set global volume
			EffectV();
			break;

		case 'W':			// W slide global volume
			EffectW();
			break;

		case 'X':			// X set panpos
			EffectX(n.dat);
			break;

		case 'Y':			// Y panbrello
			EffectY();
			break;

		case 'Z':			// Z set filter
			if(!tick){
				if(n.dat<128){
					UWORD *midimacro=m->midisfx[midiselector].macro;

					if(midimacro[0]==0xf0 && midimacro[1]==0xf0){
						if(midimacro[2]==0x01 && midimacro[3]==0x101){
							// f0 f0 01 z
							filterdamping=n.dat;
						}
						else if(midimacro[2]==0x00 && midimacro[3]==0x101){
							// f0 f0 00 z
							filtercutoff=n.dat;
						}
					}
				}
				else{
					UWORD *midimacro=m->midizxx[n.dat-128].macro;

					if(midimacro[0]==0xf0 && midimacro[1]==0xf0){
						if(midimacro[2]==0x01 && midimacro[3]<128){
							// f0 f0 01 xx
							filterdamping=(UBYTE)midimacro[3];
						}
						else if(midimacro[2]==0x00 && midimacro[3]<128){
							// f0 f0 00 xx
							filtercutoff=(UBYTE)midimacro[3];
						}
					}
				}
			}
			break;
	}
}


void MMODULE_IT::ITCOLUMN::PostEffects()
{
	if(!(n.msk&8)) return;

	switch(n.eff+'A'-1){
		case 'S':
			PostEffectSx();
			break;
	}
}


void MMODULE_IT::Update()
{
    if(!isplaying || isready) return;

	if(speed && ++tick>=(speed+extraticks)){

		extraticks=0;
		tick=0;

		if(patterndelay){
//			tick=1;
			patterndelay--;
            if(patterndelay==0){
//				tick=0;
                row++;
            }
        }
		else{
//			tick=0;
			row++;
		}

		/* if this was the last row, or if a patternbreak is active ->
		   increase the song position */

		if(row>=pattern->rows || orderjump || breaktorow){

			if(orderjump){
				songpos=(orderjump<=OrdNum) ? orderjump-1 : 0;		// v0.91 <= NOT <
				orderjump=0;

//				if(songpos==0 && loopmode==0){			FIX mikit 0.90 bug (SB0 shouldn't stop a song :)
//                  isready=1;
//                  return;
//              }
            }
            else{
                songpos++;
            }

            while(orders[songpos]==0xfe) songpos++;

            if(orders[songpos]==0xff){
                songpos=0;
                if(loopmode==0){
                    isready=1;
                    return;
                }
            }
            
            patno=orders[songpos];
            if(patno>PatNum) patno=(UBYTE)PatNum;

            pattern=&Patterns[patno];
        
            if(breaktorow){
                row=(breaktorow<=pattern->rows) ? breaktorow-1 : 0;     // v0.91 <= NOT <
                breaktorow=0;
            }
            else{
                row=0;
            }
        }

        pattern->DecodeEZRow(row,n);

//      printf("Songpos %d Patno %d Row %d\n",songpos,patno,row);
    }

    if(!scanmode){
        for(colno=0;colno<64;colno++) virt[colno].Touch();
        channelsinuse=0;
        for(colno=0;colno<64;colno++) if(virt[colno].active) channelsinuse++;
    }

    for(colno=0;colno<64;colno++){
        ITCOLUMN &curcol=cols[colno];
        ITVIRTCH *v=curcol.v;
        bool   carryvolenv=false;
        bool   carrypanenv=false;
        bool   carryptcenv=false;

        curcol.n=n[colno];

        curcol.retrig=0;
        curcol.ownvol=0;
        curcol.ownfrq=0;
        curcol.ownofs=0;
        curcol.tick=tick;

        if(chnpan[colno]!=160  && !patterndelay) curcol.ProcessTick0();
        curcol.Effects();

        if(!scanmode){
        
            if(curcol.kick){

                if(!curcol.retrig){
                    /* duplicate check ? */

                    if(Instruments[curcol.ins-1].DCT>0){
                        for(int t=0;t<64;t++) virt[t].DuplicateCheck(colno,&curcol);
                    }

                    if(v){
                        /* there is a slave for this column */

                        if(v->active){
                            /* but it's active so start the NNA and find another one */

							if(v->ins==curcol.ins){
								// active and the instruments are the same, check for envelope carry
								ITINSTRUMENT *i=&Instruments[v->ins-1];
								if(i->volenv.Flg&8) carryvolenv=true;
								if(i->panenv.Flg&8) carrypanenv=true;
								if(i->ptcenv.Flg&8) carryptcenv=true;
							}

							v->DoNNA();
							ITVIRTCH *oldv=v;
							v=vFindLeastActive();

							if(v){
								// found a new virtual channel, now perform the envelope carry
								if(carryvolenv) v->volprc=oldv->volprc;
								if(carrypanenv) v->panprc=oldv->panprc;
								if(carryptcenv) v->ptcprc=oldv->ptcprc;
							}
						}
					}
					else{
						/* there's no slave yet, so find one */
                        v=vFindLeastActive();
                    }
                }
                
                if(v){
                    /* this new slave might still be owned by another column (this happens 
                       with oneshot samples for example) so disown it first */
                    v->Disown();                
                    /* register the new owner of this slave */
                    v->owner=colno;
                    v->cut=0;
                    v->active=1;
                    v->keyon=1;
                    v->dofade=0;
                    v->background=0;
                    /* and set parameters */
                    v->nte=curcol.nte;

                    v->ins=curcol.ins;
                    v->i=(v->ins>0) ? &Instruments[v->ins-1] : (ITINSTRUMENT *)NULL;
                    
                    v->smp=curcol.smp;
                    v->s=(v->smp>0) ? &Samples[v->smp-1] : (ITSAMPLE *)NULL;
                    
                    v->startoffset=(curcol.ownofs) ? curcol.startoffset : 0;
                    v->kick=1;

                    if(!carryvolenv) v->StartVolEnv();
                    if(!carrypanenv) v->StartPanEnv();
                    if(!carryptcenv) v->StartPtcEnv();
                }

                curcol.v=v;
                curcol.kick=0;
            }

            if(v){
                v->CV =curcol.CV;
                v->NP =curcol.NP;
                v->surround=curcol.surround;
                v->vol=(curcol.ownvol) ? curcol.ovol : curcol.vol;
                v->frq=(curcol.ownfrq) ? curcol.ofrq : curcol.frq;
                v->filtercutoff=curcol.filtercutoff;
                v->filterdamping=curcol.filterdamping;

                if(v->frq<50) v->frq=50;            // 0.02B Fix : prevent divide overflow crash
            }
        }

        curcol.PostEffects();
    }

    if(patterndelrq){   
        patterndelay=patterndelrq;
        patterndelrq=0;
    }

    if(!scanmode){
        for(int t=0;t<64;t++) virt[t].Update();
    }

    driver->SetBPM(tempo);

    timestamp+=(125L*1000)/(50L*tempo);
}


int MMODULE_IT::GetTimeStamp()
{
    return timestamp;
}


void MMODULE_IT::SetTimeStamp(int ts)
{
    timestamp=ts;
}



int MMODULE_IT::GetRow()
{
    return row;
}

/*
void MMODULE_IT::SetRow(int row)
{
    for(int t=0;t<64;t++){
        if(virt[t].voice>=0) driver->VoiceStop(virt[t].voice);
    }
    breaktorow=row+1;
}


void MMODULE_IT::SetPos(int pos)
{
    if(!isplaying) return;
    for(int t=0;t<64;t++){
        if(virt[t].voice>=0) driver->VoiceStop(virt[t].voice);
    }
    orderjump=pos+1;
}
*/

