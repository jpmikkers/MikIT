/*

File:           MDRV_MIX.H
Description:    -
Version:        1.00 - original

*/
#ifndef MDRVFMX_H
#define MDRVFMX_H

#include "mtypes.h"
#include "minput.h"
#include "mdriver.h"

typedef float real;

#define MAXCHN 128
#define MAXSMP 256
#define TICKBUFFERSIZE  8192

#define DITHERSIZE  50000

#define FRACBITS 14
#define FRACMASK ((1L<<FRACBITS)-1)

#ifndef min
#define min(a,b) (((a)<(b)) ? (a) : (b))
#endif

class MDRIVER_FMX : public MDRIVER {

//  class MDCM;
    friend class MDCM;

private:

//  bool  ringenable;
//  int   ringsize;
//  int   ringpos;
//  real  *ringbufferl;
//  real  *ringbufferr;

    real  tfltbuffer[TICKBUFFERSIZE];

    SWORD *ditherl;
    int   ditherlidx;

    SWORD *ditherr;
    int   ditherridx;

    struct VOICEPARMS{
        MDRIVER_FMX *parent;
        
        int   samplehandle;

        int   keyon;
        int   islooping;
        int   isreverse;
        int   isbidi;

        int   die;
        
        UWORD flags;
        UBYTE allocated;
        UBYTE kick;
        UBYTE active;

        ULONG start;                    /* start index */
        ULONG size;                     /* samplesize */
        ULONG reppos;                   /* loop start */
        ULONG repend;                   /* loop end */

        UWORD orgflags;                 
        ULONG orgreppos;                /* loop start */
        ULONG orgrepend;                /* loop end */
        ULONG susreppos;                /* loop start */
        ULONG susrepend;                /* loop end */

        ULONG frequency;                /* current frequency */
        UBYTE volume;                   /* current volume */
        UBYTE panning;                  /* current panning position */

        double current;                 /* current index in the sample */
        double increment;               /* fixed-point increment value */
        
        real  flvolmul;                 /* left volume multiply */
        real  frvolmul;                 /* right volume multiply */

        UWORD fadecount;                /* */

        real  folvolmul;
        real  forvolmul;
        real  flvolinc;
        real  frvolinc;

        real  filtertemp1;
        real  filtertemp2;
        real  filterc1;
        real  filterc2;
        real  filterc3;

        UBYTE filtercutoff;
        UBYTE filterdamping;

        void  NewParms();
        void  UpdateIndex();

        void  Resample(void *srce,real *dest,double idx,double inc,UWORD todo);
        
        void  ResonantFilter(const real *srce,real *dest,real c1,real c2,real c3,real &tmp1,real &tmp2,int todo);
        
        void  RampMix(const real *srce,real *dest,real voll,real volr,real incl,real incr,int todo);
        void  NoRampMix(const real *srce,real *dest,real voll,real volr,int todo);

        void  MixRamp(void *srce,real *dest,double idx,double inc,UWORD todo);
        void  MixNoRamp(void *srce,real *dest,double idx,double inc,UWORD todo);

        void  AddChannel(real *buffer,UWORD todo);
    };

    friend struct VOICEPARMS;

    VOICEPARMS channel[MAXCHN];     // array of voices plus one dummy voice
    VOICEPARMS dying[MAXCHN];       // array of voices plus one dummy voice

    void    *Samples[MAXSMP];

    int     samples2bytes(int c);
    int     bytes2samples(int c); 
    int     tickleft;
    UBYTE   bpm;

    SLONG   samplesthatfit;
    real    maxfvol;

    void    WritePortion(SBYTE *buf,int todo);
    void    WriteDying(SBYTE *buf,int todo);
    void    Dither(real *buf,SWORD *dith,int &index,int skip,int todo);

protected:  
    void    WriteSamples(SBYTE *buffer,int todo);
    int     WriteBytes(SBYTE *buffer,int todo);
    void    WriteSilentSamples(SBYTE *buffer,int todo);
    int     WriteSilentBytes(SBYTE *buffer,int todo);

protected:
    ULONG   mixposition;
    real    tempbuffer[TICKBUFFERSIZE];

public:

    float   peak;

    virtual void    Init();
    virtual void    Exit();
    virtual void    Start();
    virtual void    Stop();
    virtual void    Update();

    int     AllocVoice();
    void    FreeVoice(int v);

    virtual int  PrepareSample(MSAMPLE *s);
    virtual void UnPrepareSample(int handle);

    void    VoiceSetVolume(int v,UBYTE vol);
    void    VoiceSetPanning(int v,UBYTE pan);
    void    VoiceSetFrequency(int v,ULONG frq);
    void    VoiceSetFilter(int v,UBYTE cutoff,UBYTE damping);
    void    VoicePlay(int voice,int handle,ULONG start,ULONG size,ULONG reppos,ULONG repend,ULONG sreppos,ULONG srepend,UWORD flags);
    void    VoiceStop(int voice);
    int     VoiceActive(int voice);
    void    VoiceKeyOn(int voice);
    void    VoiceKeyOff(int voice);
    
    UBYTE   GetBPM();
    void    SetBPM(UBYTE bpm);

    ULONG   GetMixPosition();

//  void    GetScopeData(SBYTE data[2][600]);

    MDRIVER_FMX();
    virtual ~MDRIVER_FMX();
};

#endif
