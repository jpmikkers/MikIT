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

File:           MMOD_IT.H
Description:    -
Version:        1.00 - original
                1.01 - high offset command SAx back 
*/
#ifndef MMOD_IT_H
#define MMOD_IT_H

#include "mtypes.h"
#include "minput.h"
#include "mmodule.h"
#include "mdriver.h"



/****************************************************************************
 MMODULE_IT class :
****************************************************************************/


class MMODULE_IT : public MMODULE {

/************************* LOADER STUFF *************************/

    private:

        class ITMIDI{
            public:
            UWORD macro[32];
            void  Load(MINPUT *in);
            void  DefaultSFX();
            void  DefaultZXX(UBYTE v);
        };
        
        
        struct ITNOTESAMPLE{
            UBYTE note;
            UBYTE sample; 
        };


        class ITNOTE{
            public:
                
            UBYTE msk;
            UBYTE nte;
            UBYTE ins;
            UBYTE vol;
            UBYTE eff;
            UBYTE dat;

            void ITDecode(UBYTE * &p,UBYTE chv,ITNOTE &previous);
            int  EZCount();
            void EZEncode(UBYTE * &d,int channel);
            void EZDecode(UBYTE * &p);

            ITNOTE();
        };



        class ITENVELOPE {
                public:
                UBYTE Flg;
                UBYTE Num;
                UBYTE LpB;
                UBYTE LpE;
                UBYTE SLB;
                UBYTE SLE;

                struct{
                        UBYTE value;
                        UWORD tick;
                } points[25];                          

                ITENVELOPE();
                SWORD Interpolate(SWORD p,SWORD p1,SWORD p2,SWORD v1,SWORD v2);
                void  Load(MINPUT *in);
                void  LoadOld(MINPUT *in,UBYTE iFlg,UBYTE iLpB,UBYTE iLpE,UBYTE iSLB,UBYTE iSLE);
                virtual void Expand()=0;
        };
        

        class ITVOLENV : public ITENVELOPE {
            public:
                UBYTE *values;

                void  Expand();

                ITVOLENV();
                ~ITVOLENV();
        };


        class ITPANENV : public ITENVELOPE {
            public:
                SBYTE *values;

                void  Expand();

                ITPANENV();
                ~ITPANENV();
        };


        class ITPTCENV : public ITENVELOPE {
            public:
                SWORD *values;

                void  Expand();

                ITPTCENV();
                ~ITPTCENV();
        };


        class ITPROCESS {
            private:
            UWORD LpB;
            UWORD LpE;
            UWORD SLB;
            UWORD SLE;
            UWORD tick;

            public:
            UWORD Lst;
            UBYTE Flg;

            ITPROCESS();

            void  Start(ITENVELOPE *e);
            UWORD GetTick();
            void  Process(int keyon);
            int   Done();
        };


        class ITINSTRUMENT {
                public:
            
                UBYTE filename[13];
                UBYTE NNA,DCT,DCA;
                UWORD FadeOut;
                UBYTE PPS,PPC,GbV,DfP,RV,RP;
                UWORD TrkVers;
                UBYTE NoS;
                UBYTE defaultcutoff,defaultdamping;

                char  insname[26+1];
                ITNOTESAMPLE notesample[120];

                ITVOLENV volenv;
                ITPANENV panenv;
                ITPTCENV ptcenv;

                void  Load(MINPUT *i);
                void  LoadOld(MINPUT *i);
                void  CreateFromSample(int smpno);
                UBYTE GetGbV();
                UBYTE RandomPan(UBYTE pan);
        };


        class ITPATTERN {
            
            private:

                void    OneRow(UBYTE * &in,ITNOTE n[64],ITNOTE p[64]);
                int     Precode(UBYTE *in);
                void    Recode(UBYTE *in,UBYTE *out);           
                UBYTE * DecodeEZRow(UBYTE *p,ITNOTE n[64]);

                UBYTE *data;
                UWORD *indx;

            public:
                UWORD rows;
                void Load(MINPUT *i,long offset);
                void DecodeEZRow(int row,ITNOTE n[64]);

                ITPATTERN();
                ~ITPATTERN();
        };


        class ITSAMPLE {
//                public:
//              MDRIVER *driver;
                
                public:
                int     handle;
                MSAMPLE *msample;
                    
                UBYTE   filename[13];
                UBYTE   GvL;
                UBYTE   Flg;
                UBYTE   Vol;
                char    smpname[26+1];
                UBYTE   Cvt;
                UBYTE   DfP;
              
                ULONG   Length;
                ULONG   LoopBeg,LoopEnd;
                ULONG   C5Speed;
                ULONG   SLoopBeg,SLoopEnd;
                ULONG   SamplePtr;
                UBYTE   ViS,ViD,ViR,ViT;

                void    Load(MINPUT *i);

                UWORD   mmFlg;

                ITSAMPLE();
                ~ITSAMPLE();
        };


        // module header:

        char      songname[26+1];         /* the songname.. */
        UWORD     OrdNum;
        UWORD     InsNum;
        UWORD     SmpNum;
        UWORD     PatNum;
        UWORD     Cwtv;
        UWORD     Cmwt;
        UWORD     Flags;
        UWORD     Special;

        UBYTE     GV;
        UBYTE     MV;
        UBYTE     IS;
        UBYTE     IT;
        UBYTE     Sep;

        UWORD     MsgLgth;
        ULONG     MsgOffs;

        UBYTE     chnpan[64];
        UBYTE     chnvol[64];

        UBYTE    *orders;
        ULONG    *insoffsets;
        ULONG    *smpoffsets;
        ULONG    *patoffsets;

        char     *Message;

        ITMIDI    midiini[9];
        ITMIDI    midisfx[16];
        ITMIDI    midizxx[128];
        
        ITINSTRUMENT *Instruments;
        ITSAMPLE *Samples;
        ITPATTERN *Patterns;
        
        void LoadHeader();
        void LoadInstrument(ITINSTRUMENT *i);

        /*** modplaying variables ***/

        struct ITVIRTCH;
        
        struct ITCOLUMN{
            MMODULE_IT *m;                                  

            ITNOTE n;
            UBYTE tick;         
            UBYTE panslidespd;
            UBYTE volslidespd;
            UBYTE chvolslidespd;
            UBYTE pitchslidespd;
            UBYTE temposlidespd;
            UBYTE gvolslidespd;

            UBYTE vvolslidespd;
            UBYTE vpitchslidespd;

            UBYTE qptr;
            UBYTE qspeed;

            ULONG destfrq;

            ITVIRTCH *v;
            int   kick;
            int   ownvol;
            int   ownfrq;
            int   ownofs;
            int   retrig;

            UBYTE nte;
            UBYTE ins;
            UBYTE vol;
//          UBYTE pan;
            ULONG frq;
            UBYTE ovol;
            ULONG ofrq;
            UBYTE smp;
            UBYTE smpnote;
            UBYTE CV;       // channel volume
            UBYTE CP;       // channel panning
            UBYTE NP;       // note panning

            UBYTE surround; // surround flag

            UBYTE tremorspd;
            UBYTE tremorptr;
            UBYTE arpeggio;

            UBYTE vibspd;
            UBYTE vibdpt;
            UBYTE vibtyp;
            UBYTE vibptr;

            UBYTE trmspd;
            UBYTE trmdpt;
            UBYTE trmtyp;
            UBYTE trmptr;

            UBYTE highoffset;           /* doesn't seem to work in impulse tracker? BACK in : 0.07 */
			ULONG startoffset; 

			UBYTE loopbackpoint;
			UBYTE loopbackcount;

			UBYTE sfxdata;

			UBYTE filtercutoff;
			UBYTE filterdamping;

			UBYTE midiselector;
			
			void  FindSmpNoteFreq();
			void  ProcessTick0();
			void  VolSlideUp(UBYTE v);
			void  VolSlideDown(UBYTE v);
			void  PanSlideLeft(UBYTE v);
			void  PanSlideRight(UBYTE v);
			void  FrqSlideUp(UBYTE v);
			void  FrqSlideDown(UBYTE v);
			void  DoVibrato();

			void  SetParent(MMODULE_IT *im);
			void  Effects();
			void  PostEffects();

			void  VEffectA(UBYTE dat);
			void  VEffectB(UBYTE dat);
			void  VEffectC(UBYTE dat);
			void  VEffectD(UBYTE dat);
			void  VEffectE(UBYTE dat);
			void  VEffectF(UBYTE dat);
			void  VEffectG(UBYTE dat);
			void  VEffectH(UBYTE dat);

			void  EffectA();
			void  EffectB();
			void  EffectC();
			void  EffectD();
			void  EffectE();
			void  EffectF();
			void  EffectG(UBYTE dat);
			void  EffectH();
			void  EffectI();
			void  EffectJ();
//			void  EffectK();
//			void  EffectL();
			void  EffectM();
			void  EffectN();
			void  EffectO();
			void  EffectP();
			void  EffectQ();
			void  EffectR();
			void  EffectSx();
			void  PostEffectSx();
			void  EffectSB(UBYTE dat);
			void  EffectT();
			void  EffectU();
			void  EffectV();
			void  EffectW();
			void  EffectX(UBYTE dat);
			void  EffectY();
		};

		struct ITVIRTCH{
			MMODULE_IT *m;									

			int	  owner;	
			int	  active;
			int   background;
			int   voice;
			int	  kick;
			int	  keyon;
			int   dofade;
			int	  cut;
			UBYTE NNA;
			UBYTE nte;
			UBYTE ins;
			UBYTE smp;
	        ITINSTRUMENT *i;
		    ITSAMPLE *s;
			UBYTE vol;
//			UBYTE pan;
			ULONG frq;
			UWORD NFC;
			UBYTE SV;
			UBYTE IV;
			ITPROCESS volprc;
			ITPROCESS panprc;
			ITPROCESS ptcprc;
			UBYTE CV;		// channel volume
			UBYTE NP;		// note panning
			UBYTE surround;	// surround flag
			UBYTE VEV;		// envelope volume

			UBYTE vibrunp;	// vibrato running pointer
			UWORD vibrund;	// vibrato running depth

			ULONG startoffset;

			UBYTE filtercutoff;
			UBYTE filterdamping;

			void SetParent(MMODULE_IT *im);
			void Update();
			void Touch();
			void Disown();
			void DoNNA(UBYTE nna);
			void DoNNA();
			void DuplicateCheck(int column,ITCOLUMN *c);
			void PastNoteAction(int column,UBYTE action);
			void StartVolEnv();
			void StartPanEnv();
			void StartPtcEnv();
		};

		int    timestamp;
		
		protected:
	
		static ULONG PitchTable[10*12];
		static ULONG FineLinearSlideUpTable[16];
		static ULONG FineLinearSlideDownTable[16];
		static ULONG LinearSlideUpTable[257];
		static ULONG LinearSlideDownTable[257];
		static SBYTE VibTables[3][256];
		static UBYTE VolFxTable[10];

		friend struct ITCOLUMN;	
		friend struct ITVIRTCH;	

		ITVIRTCH   virt[64];
		ITCOLUMN   cols[64];

		UBYTE	   runGV;
		int		   row;
		UBYTE	   patterndelay;
		UBYTE      patterndelrq;
		UBYTE	   speed,tick,extraticks;
		UBYTE	   tempo;
		UBYTE	   songpos;
		UBYTE	   patno;

		UWORD	   breaktorow;
		UWORD	   orderjump;

		ITPATTERN *pattern;
		ITNOTE	   n[64];

		int		   colno;

		void       ProcessTick0(ITCOLUMN &c,ITNOTE &n);
		void	   FindSmpNoteFreq(ITCOLUMN &c);
		ITVIRTCH  *vFindLeastActive();

public:
//		UBYTE	dbg1;
		int		channelsinuse;
		int		isready;
		
        static int Test(MINPUT *i,char *title,int maxlen);

//		void GetSongTitle(char *s,int l);
		int	 GetSongMessage(char *d,int l);
//		int  GetSampleName(char *d,int l,int i);
//		int	 GetInstrumentName(char *d,int l,int i);

		int GetNumSamples();
		int GetNumInstruments();
		char *GetSongTitle();
		char *GetSampleName(int s);
		char *GetInstrumentName(int i);

        MMODULE_IT();
        MMODULE_IT(MINPUT *i);
        ~MMODULE_IT();

/************************* PLAYER STUFF *************************/

private:
        int isplaying;

public:
		void Load(MINPUT *i);
        void Start(MDRIVER *d);
        void Stop();
        void Update();
		void Restart();
		int  GetPos();
        int  GetRow();
		int  GetSongLength();
		void PrevPos();
		void NextPos();
		int  IsReady();
		int  GetNumChannels();
		
		int  GetTimeStamp();
		void SetTimeStamp(int ts);
};


#endif
