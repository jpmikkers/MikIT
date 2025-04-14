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
#ifndef MMOD_XM_H
#define MMOD_XM_H

#include "mtypes.h"
#include "minput.h"
#include "mmodule.h"
#include "mdriver.h"

#define XMHEADERSIZE 336
#define XMPATTERNHEADERSIZE 9
#define XMINSTRUMENTHEADERSIZE 29
#define XMPATCHHEADERSIZE 214
#define XMSAMPLEHEADERSIZE 40

/* song flags: */

#define XM_LINEAR   0x1


/* sample flags: */

#define XM_LOOP     0x1
#define XM_BIDI     0x2
#define XM_16BITS   0x10


/* Envelope flags: */

#define XE_ON       1
#define XE_SUSTAIN  2
#define XE_LOOP     4



/****************************************************************************
 MMODULE_XM class :
****************************************************************************/

class MMODULE_XM : public MMODULE {

/************************* LOADER STUFF *************************/

    private:
        
        struct XMENVTUPLE {
            UWORD pos,val;
        };

        class XMENV{
            public:
            UBYTE *envelope;
            UWORD points;
            UWORD sus,beg,end;
            UBYTE flag;

            SWORD Interpolate(SWORD p,SWORD p1,SWORD p2,SWORD v1,SWORD v2);
            void Expand(XMENVTUPLE tuples[12],UBYTE numtuples,UBYTE iflag,UBYTE isus,UBYTE ibeg,UBYTE iend);

            XMENV();
            ~XMENV();
        };

        class XMSAMPLE{
            public:

            ULONG bytelength;       /* (dword) Sample length in bytes */
            ULONG length;           /* Sample length (in samples) */
            ULONG loopstart;        /* sample loop start */
            ULONG loopend;          /* sample loop end */
            UBYTE volume;           /* (byte) Volume */
            SBYTE finetune;         /* (byte) Finetune (signed byte -128..+127) */
            UBYTE panning;          /* (byte) Panning (0-255) */
            SBYTE relnote;          /* (byte) Relative note number (signed byte) */
            UBYTE samplename[22];   /* (char) Sample name */

            MSAMPLE *msample;       
            SWORD handle;           /* sample handle to physical sample data */
            UWORD mmFlg;

            void LoadHeader(MINPUT *in);
            void LoadBody(MINPUT *in);

            XMSAMPLE();
            ~XMSAMPLE();
        };
        
        class XMINSTRUMENT{

            public:

            char  name[22+1];       /* (char) Instrument name */
            UWORD numsmp;           /* (word) Number of samples in instrument */

            UBYTE what[96];         /* (byte) Sample number for all notes */

            XMENV volenv;           /* volume envelope */
            XMENV panenv;           /* panning envelope */
            
            UBYTE vibflg;           /* (byte) Vibrato type */
            UBYTE vibsweep;         /* (byte) Vibrato sweep */
            UBYTE vibdepth;         /* (byte) Vibrato depth */
            UBYTE vibrate;          /* (byte) Vibrato rate */
            UWORD volfade;          /* (word) Volume fadeout */

            XMSAMPLE *samples;      /* all samples */

            void Load(MINPUT *i);
            void dummy();
            void Connect(MDRIVER *d);
            void Disconnect(MDRIVER *d);

            XMINSTRUMENT();
            ~XMINSTRUMENT();
        };

        class XMPATTERN {
            public:
            UWORD numrows;                  /* (word) Number of rows in pattern (1..256) */
            UBYTE *data;                    /* pattern data */
            UWORD *index;                   /* table of indexes */

            XMPATTERN();
            ~XMPATTERN();

            void Load(MINPUT *i,UWORD numchn);
            void dummy();
        };

        class AUDT {
            public:

            MMODULE_XM *parent;

            int voice;

            UBYTE tick;
            UBYTE fetch;
            
            XMINSTRUMENT *i;
            XMSAMPLE *s;

            UWORD fadevol;      /* fade volume */

            SBYTE finetune;     /* current sample finetune */

            UBYTE note;         /* current note (needed for arpeggio - fx 0)*/
            UBYTE volume;       /* channel (base-)volume */
            UWORD period;       /* channel (base-)period */
            UBYTE realvolume;
            UWORD realperiod;

            UBYTE perdropback;  /* period drop-back flag */

            UBYTE portupspd;    /* fx 1 (portamento up) data */
            UBYTE portdnspd;    /* fx 2 (portamento dn) data */
            UBYTE fportupspd;   /* fx E1 (fine portamento up) data */
            UBYTE fportdnspd;   /* fx E2 (fine portamento dn) data */
            UBYTE ffportupspd;  /* fx X1 (extra fine portamento up) data */
            UBYTE ffportdnspd;  /* fx X2 (extra fine portamento dn) data */

            UWORD destperiod;   /* fx 3 (noteslide) destiny period */
            UBYTE noteslspd;    /* fx 3 speed */

            UBYTE vibspd;       /* fx 4 speed */
            UBYTE vibdpt;       /* fx 4 depth */
            UBYTE vibpos;       /* fx 4 pos */
            UBYTE vibctl;       /* fx 4 ctl */

            UBYTE trmspd;       /* fx 7 speed */
            UBYTE trmdpt;       /* fx 7 depth */
            UBYTE trmpos;       /* fx 7 pos */
            UBYTE trmctl;       /* fx 7 ctl */

            UBYTE altstart;     /* fx 9 data: alternate start position flag */
            ULONG startpos;     /* fx 9 data: start position */

            UBYTE volslspd;     /* fx A speed */
            UBYTE fvolupspd;    /* fx EA speed */
            UBYTE fvoldnspd;    /* fx EB speed */

            UBYTE panning;  /* channel panning */
            UBYTE panslspd;

            UBYTE kick;
            UBYTE keyon;

            UWORD venvpos;  /* volume envelope position */
            UWORD penvpos;

            UWORD avibpos;  /* autovibrato pos */
            UWORD aswppos;  /* autovibrato sweep pos */

            UBYTE mtrigspd;
            UBYTE mtrigcnt;
            UBYTE mtrigfad;

            UBYTE tremorcnt;
            UBYTE tremorspd;

            UBYTE filtercutoff;
            UBYTE filterdamping;

            void HandleTick(UBYTE nte,UBYTE ins,UBYTE vol,UBYTE eff,UBYTE dat);
            void SetVolume(SBYTE ivol);
            void ChangeVolume(SBYTE ivol);
            void SetPeriod(UWORD iperiod);
            void ChangePeriod(UWORD iperiod);
            void KeyOn();
            void HandleVolEffects(UBYTE vol);
            void HandleStdEffects(UBYTE vol,UBYTE eff,UBYTE dat);
            void HandleEEffects(UBYTE eff,UBYTE dat);
            void DoNoteSlide();
            void DoVolSlide();
            void DoVibrato();
            void DoTremolo();
            void DoMultiRetrig();
        };

        // module header:
//      char  id[17];                   /* ID text: 'Extended module: ' */
        char  songname[20+1];           /* Module name, padded with zeroes and 0x1a at the end */
//      char  eof;                      /* */
        UBYTE trackername[20];          /* Tracker name */
//      UWORD version;                  /* (word) Version number, hi-byte major and low-byte minor */
//      ULONG headersize;               /* Header size */
        UWORD songlength;               /* (word) Song length (in patten order table) */
        UWORD restart;                  /* (word) Restart position */
        UWORD numchn;                   /* (word) Number of channels (2,4,6,8,10,...,32) */
        UWORD numpat;                   /* (word) Number of patterns (max 256) */
        UWORD numins;                   /* (word) Number of instruments (max 128) */
        UWORD flags;                    /* (word) Flags: bit 0: 0 = Amiga frequency table (see below) 1 = Linear frequency table */
        UWORD defaultspeed;             /* (word) Default speed */
        UWORD defaultbpm;               /* (word) Default BPM */
        UBYTE orders[256];              /* (byte) Pattern order table */

        XMPATTERN *patterns;            /* all patterns */
        XMINSTRUMENT *instruments;

        /*******************************/

        AUDT  audio[32];

        UBYTE speed,tick,bpm;
        XMPATTERN *pattern;

        UWORD breakpos;     /* pattern break position */
        UWORD posjump;      /* position jump position */
        UWORD looppos;      /* pattern loop position */
        UWORD loopcnt;      /* pattern loop count */

        SBYTE globalvolume;
        UBYTE globalslide;

        UBYTE patdelayc;
        UBYTE patdelayd;

        SWORD songpos;      /* current song position */
        SWORD row;          /* current row */

        UBYTE isready;
        
        XMSAMPLE dummysample0;
        XMSAMPLE dummysample1;
//      XMPATTERN dummypattern;
//      XMINSTRUMENT dummyinstrument;

        XMINSTRUMENT *GetInstrument(UBYTE ins);
        XMSAMPLE *GetSample(XMINSTRUMENT *i,UBYTE note);
        UWORD  getperiod(UBYTE note,SBYTE fine);
        ULONG  getfrequency(UWORD period);
        SWORD  DoPan(SWORD envpan,SWORD pan);
        UBYTE  ProcessEnvelope(UWORD *position,UBYTE keyon,UBYTE flags,UWORD points,UWORD sus,UWORD beg,UWORD end,UBYTE *table);
        UBYTE *FetchNIVED(UBYTE *p,UBYTE &nte,UBYTE &ins,UBYTE &vol,UBYTE &eff,UBYTE &dat);

        friend class XMINSTRUMENT;
        friend class XMSAMPLE;
        friend class AUDT;

        int    timestamp;

public:
        static int Test(MINPUT *i,char *title,int maxlen);
        void Load(MINPUT *i);

        char *GetInstrumentName(int i);
        int  GetNumInstruments();
        char *GetSongTitle();
        int  GetSongLength();
        int  IsReady();
        int  GetNumChannels();

        MMODULE_XM();
        ~MMODULE_XM();

/************************* PLAYER STUFF *************************/

private:
        int isplaying;

public:
        void Start(MDRIVER *d);
        void Stop();
        void Update();
        void Restart();
        int  GetPos();
                int  GetRow();
        void PrevPos();
        void NextPos();

        int  GetTimeStamp();
        void SetTimeStamp(int ts);
};


#endif
