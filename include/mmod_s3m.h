/*

*/
#ifndef MMOD_S3M_H
#define MMOD_S3M_H

#include "mtypes.h"
#include "minput.h"
#include "mmodule.h"
#include "mdriver.h"

/****************************************************************************
 MMODULE_S3M class :
****************************************************************************/


class MMODULE_S3M : public MMODULE {

/************************* LOADER STUFF *************************/

private:

    struct S3MNOTE{
        UBYTE nte,ins,vol,eff,dat;
    };
    
    class S3MPATTERN{
        public:

        UBYTE *data;
        UWORD index[64];

        UBYTE *FakeRow(UBYTE *p);
        void  Fake();
        void  DecodeRow(int row,S3MNOTE notes[32]);
        void Load(MINPUT *i,ULONG seekpos);

        S3MPATTERN();
        ~S3MPATTERN();
    };

    class S3MSAMPLE{       /* sample header as it appears in a module */

    public:
        
        char  name[28+1];
    
        ULONG length;
        ULONG loopstart;
        ULONG loopend;
        UWORD flags;
        UBYTE volume;
        ULONG c2spd;

        // extra information:

        int  handle;
        MSAMPLE *msample;

        void Load(MINPUT *i,ULONG seekpos);

        S3MSAMPLE();
        ~S3MSAMPLE();
    };

    class AUDTMP{
        public:

        MMODULE_S3M *parent;

        int   voice;

        UBYTE tick;

        UBYTE kick;         /* if true=sample has to be restarted */
        UBYTE cut;          /* if true -> sample has to be cut */

        UBYTE sample;       /* which sample number (0-31) */

        ULONG startoffset;  /* The start byte index in the sample */

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
        SWORD ownofs;

        UBYTE volslidespd;
        UWORD pitchslidespd;

        SBYTE retrig;       /* retrig value (0 means don't retrig) */
		ULONG c2spd;		/* what finetune to use */

		SBYTE tmpvolume;	/* tmp volume */

		UWORD tmpperiod;	/* tmp period */
		UWORD wantedperiod;	/* period to slide to (with effect 3 or 5) */

		UWORD slidespeed;	/* */
		UWORD portspeed;	/* noteslide speed (toneportamento) */

		UBYTE glissando;	/* glissando (0 means off) */
		UBYTE wavecontrol;	/* */

		UBYTE s3mtremor;
		UBYTE tremorspd;

		UBYTE vibspd;
		UBYTE vibdpt;
//		UBYTE vibtyp;
		UBYTE vibptr;

		UBYTE trmptr;		/* current tremolo position */
		UBYTE trmspd;		/* "" speed */
		UBYTE trmdpt;		/* "" depth */

		UBYTE s3mrtgslide;
		UBYTE s3mrtgspeed;

		UBYTE arpeggio;

		void DoTremolo();
		void DoVibrato(UBYTE vibspd,UBYTE vibdpt);
		void HandleEEffects(UBYTE hi,UBYTE lo);
		void HandleEffects(UBYTE eff,UBYTE dat);
		void HandleTick(UBYTE nte,UBYTE ins,UBYTE vol,UBYTE eff,UBYTE dat);
		void DoToneSlide();
		void DoVolSlide();
		void VolSlideUp(UBYTE v);
		void VolSlideDown(UBYTE v);

		void DoPitchSlideDown();
		void DoPitchSlideUp();
		
		void EffectG(UBYTE dat);
		void DoS3MRetrig(UBYTE inf);
		void EffectSx(UBYTE dat);
	};

	AUDTMP	audio[16];
	
	// module header:

    char    songname[28+1];       /* the songname.. */
    UWORD   songlength;           /* number of patterns used */
    UBYTE   positions[256];	 	  /* which pattern to play at pos */

	UBYTE	s3mtype;
	UWORD	s3mflags;
	UWORD	tracker;

	UBYTE	mastervol;
	UBYTE   initspeed;
	UBYTE	inittempo;
	UBYTE	mastermult;

	UBYTE	channels[32];
	UBYTE	panning[32];
	UBYTE   remap[32];

    // extrapolated info

    UBYTE     numchn;
    UWORD     numpat;
	UWORD	  numsmp;
//  UWORD     numtrk;        

	// parapointers

	UWORD	  *smppara;
	UWORD	  *patpara;


    // patterns

    S3MPATTERN *patterns;
	S3MSAMPLE  *samples;

	/***********************/
	
	UBYTE speed,tick,bpm;

	UBYTE patdly,patdly2;
	UBYTE repcnt,reppos;
	UBYTE globalvolume;

	friend class AUDTMP;
	int    timestamp;

public:
    static int Test(MINPUT *i,char *title,int maxlen);
    void Load(MINPUT *i);
	
    MMODULE_S3M();
    ~MMODULE_S3M();

/************************* PLAYER STUFF *************************/

private:
	UBYTE isready;
    int isplaying;

	UWORD songpos;
	
	SWORD row;

	UBYTE breakpos;
	UBYTE posjump;

public:
    void Start(MDRIVER *d);
    void Stop();
    void Update();
	void Restart();

	char *GetSongTitle();		
	char *GetSampleName(int n);

	int  GetNumChannels();
	int	 GetNumSamples();
	
	int GetSongLength();
	int GetPos();
	void NextPos();
	void PrevPos();
	int IsReady();
//	void GetSongTitle(char *d,int l);
//	int GetSampleName(char *d,int l,int i);
	int  GetTimeStamp();
	void SetTimeStamp(int ts);
};

#endif
