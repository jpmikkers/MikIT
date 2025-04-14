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

*/
#ifndef MMODMOD_H
#define MMODMOD_H

#include "mtypes.h"
#include "minput.h"
#include "mmodule.h"
#include "mdriver.h"

/****************************************************************************
 MMODULE_MOD class :
****************************************************************************/


class MMODULE_MOD : public MMODULE {

/************************* LOADER STUFF *************************/

private:

    struct MODTYPE {
        UBYTE id[5];
        UWORD channels;
    };

    static MODTYPE modtypes[22];

    struct MODNOTE{
        UBYTE ins,nte,eff,dat;
    };

    class MODPATTERN{
        public:

        MODNOTE *data;

        void Load(MINPUT *i,UWORD numchn);

        MODPATTERN();
        ~MODPATTERN();
    };

    class MODSAMPLE{       /* sample header as it appears in a module */
    
    public:
        
        char  name[22+1];
    
        ULONG length;
        ULONG loopstart;
        ULONG loopend;
        UWORD flags;
        UBYTE finetune;
        UBYTE volume;

        // extra information:

        int  handle;
        MSAMPLE *msample;

        void LoadHeader(MINPUT *i);
        void LoadBody(MINPUT *i);

        MODSAMPLE();
        ~MODSAMPLE();
    };

    class AUDTMP{
        public:

        MMODULE_MOD *parent;

        int   voice;

        UBYTE tick;

        UBYTE kick;         /* if true=sample has to be restarted */
        UBYTE sample;       /* which sample number (0-31) */
        SWORD handle;       /* which sample-handle */

        ULONG start;        /* The start byte index in the sample */

        UBYTE panning;      /* panning position */
        SBYTE volume;       /* amiga volume (0 t/m 64) to play the sample at */
        UWORD period;       /* period to play the sample at */

        UBYTE finetune;

        /* You should not have to use the values
           below in the player routine */

        SBYTE transpose;

        UBYTE note;         /* */

        SWORD ownper;
        SWORD ownvol;

        SBYTE retrig;       /* retrig value (0 means don't retrig) */
		UWORD c2spd;		/* what finetune to use */

		SBYTE tmpvolume;	/* tmp volume */

		UWORD tmpperiod;	/* tmp period */
		UWORD wantedperiod;	/* period to slide to (with effect 3 or 5) */

		UWORD slidespeed;	/* */
		UWORD portspeed;	/* noteslide speed (toneportamento) */

		UBYTE glissando;	/* glissando (0 means off) */
		UBYTE wavecontrol;	/* */

		SBYTE vibpos;		/* current vibrato position */
		UBYTE vibspd;		/* "" speed */
		UBYTE vibdepth;		/* "" depth */

		SBYTE trmpos;		/* current tremolo position */
		UBYTE trmspd;		/* "" speed */
		UBYTE trmdepth;		/* "" depth */

		UWORD soffset;		/* last used sample-offset (effect 9) */

		void DoTremolo();
		void DoVibrato();
		void HandleEEffects(UBYTE hi,UBYTE lo);
		void HandleEffects(UBYTE eff,UBYTE dat);
		void HandleTick(UBYTE nte,UBYTE ins,UBYTE eff,UBYTE dat);
		void DoToneSlide();
		void DoVolSlide(UBYTE dat);
	};
	
	AUDTMP	  audio[32];
	
	// module header:

    char      songname[20+1];       /* the songname.. */
    UBYTE     songlength;           /* number of patterns used */
    UBYTE     positions[128];       /* which pattern to play at pos */

    // extrapolated info

    UWORD     numchn;
    UWORD     numpat;
	UWORD	  numsmp;
//  UWORD     numtrk;        

    // patterns

    MODPATTERN *patterns;
	MODSAMPLE  *samples;

	/***********************/
	
	UBYTE speed,tick,bpm;
//	MODPATTERN *pattern;

	UWORD breakpos;		/* pattern break position */
	UWORD posjump;		/* position jump position */
	UWORD looppos;		/* pattern loop position */
	UWORD loopcnt;		/* pattern loop count */

	SBYTE globalvolume;
	UBYTE globalslide;

	UBYTE patdly;
	UBYTE patdly2;

	SWORD songpos;		/* current song position */
	SWORD row;          /* current row */

	UBYTE isready;

    void LoadPattern(MODNOTE *);

	friend class AUDTMP;

	private:

	int    timestamp;

public:
    static int Test(MINPUT *i,char *title,int maxlen);
    void Load(MINPUT *i);
//    static MMODULE *Load(MINPUT *i,bool (*terminator)(void *),void *);
	
    MMODULE_MOD();
    ~MMODULE_MOD();

/************************* PLAYER STUFF *************************/

private:
    int isplaying;
    int channel[32];

public:
    void Start(MDRIVER *d);
    void Stop();
    void Update();
	void Restart();

	int GetNumChannels();
	int GetSongLength();
	int GetPos();
	void NextPos();
	void PrevPos();
	int IsReady();
	char *GetSongTitle();
	char *GetSampleName(int s);
	int  GetNumSamples();

	int  GetTimeStamp();
	void SetTimeStamp(int ts);
};

#endif
