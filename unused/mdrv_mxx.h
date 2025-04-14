/*

File:           MDRV_MIX.H
Description:    -
Version:        1.00 - original

*/
#ifndef MDRVMXX_H
#define MDRVMXX_H

#include "mtypes.h"
#include "minput.h"
#include "mdriver.h"


#define MAXCHN 128
#define MAXSMP 256
#define TICKBUFFERSIZE  8192

#define FRACBITS 11
#define FRACMASK ((1L<<FRACBITS)-1)

#ifndef min
#define min(a,b) (((a)<(b)) ? (a) : (b))
#endif

class MDRIVER_MXX : public MDRIVER {

//  class MDCM;
    friend class MDCM;

private:

    SLONG tickbuffer[TICKBUFFERSIZE];

    struct VOICEPARMS{
        MDRIVER_MXX *parent;
        
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
        SLONG current;                  /* current index in the sample */
        SLONG increment;                /* fixed-point increment value */
        SLONG lvolmul;                  /* left volume multiply */
        SLONG rvolmul;                  /* right volume multiply */

        UWORD fadecount;                /* */
        SLONG olvolmul;                 /* left volume multiply */
        SLONG orvolmul;                 /* right volume multiply */
        SLONG lvolinc;
        SLONG rvolinc;
        
        void NewParms();
        SLONG fraction2long(ULONG dividend,UWORD divisor);
    };

    friend VOICEPARMS;

    SLONG   amplifytable[MAXCHN];       // array of amplification factors where 65536 = 1.0 
    
    VOICEPARMS channel[MAXCHN];     // array of voices plus one dummy voice
    VOICEPARMS dying[MAXCHN];       // array of voices plus one dummy voice

        void    *Samples[MAXSMP];

        int     samples2bytes(int c);
        int     bytes2samples(int c); 
        int     tickleft;
    UBYTE   bpm;

    SLONG   maxvol,samplesthatfit;
        int     ampshift;

    void    Mix16StereoNoclick(SWORD *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG lvolmul,SLONG rvolmul,SLONG lvolinc,SLONG rvolinc);
    void    Mix08StereoNoclick(SBYTE *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG lvolmul,SLONG rvolmul,SLONG lvolinc,SLONG rvolinc);
    void    Mix16MonoNoclick(SWORD *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG volmul,SLONG volinc);
    void    Mix08MonoNoclick(SBYTE *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG volmul,SLONG volinc);

    void    Mix16StereoNormal(SWORD *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG lvolmul,SLONG rvolmul);
    void    Mix16StereoInterp(SWORD *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG lvolmul,SLONG rvolmul);
    void    Mix16MonoNormal(SWORD *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG volmul);
    void    Mix16MonoInterp(SWORD *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG volmul);
    
    void    Mix08StereoNormal(SBYTE *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG lvolmul,SLONG rvolmul);
    void    Mix08StereoInterp(SBYTE *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG lvolmul,SLONG rvolmul);
    void    Mix08MonoNormal(SBYTE *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG volmul);
    void    Mix08MonoInterp(SBYTE *srce,SLONG *dest,SLONG index,SLONG increment,UWORD todo,SLONG volmul);

    UWORD   NewPredict(SLONG index,SLONG end,SLONG increment,UWORD todo);
    void    Sample32To8Copy(SLONG *srce,SBYTE *dest,ULONG count,UBYTE shift);
    void    Sample32To16Copy(SLONG *srce,SWORD *dest,ULONG count,UBYTE shift);
    void    WritePortion(SBYTE *buf,int todo);
    void    WriteDying(SBYTE *buf,int todo);
    void    AddChannel(VOICEPARMS *vnf,UWORD todo);

protected:  
    void    WriteSamples(SBYTE *buffer,int todo);
        int     WriteBytes(SBYTE *buffer,int todo);
    void    WriteSilentSamples(SBYTE *buffer,int todo);
        int     WriteSilentBytes(SBYTE *buffer,int todo);

public:

#ifdef PEAKDETECTION
    SLONG   peak;
#endif
    
    virtual void    Init();
        virtual void    Exit();
        virtual void    Start();
        virtual void    Stop();
/*  virtual void    Update();
*/
    int     AllocVoice();
    void    FreeVoice(int v);

    virtual int     SampleLoad(MINPUT *i,ULONG length,ULONG reppos,ULONG repend,UWORD flags);
    virtual void    SampleFree(int handle);

    void    VoiceSetVolume(int v,UBYTE vol);
    void    VoiceSetPanning(int v,UBYTE pan);
    void    VoiceSetFrequency(int v,ULONG frq);
    void    VoicePlay(int voice,int handle,ULONG start,ULONG size,ULONG reppos,ULONG repend,ULONG sreppos,ULONG srepend,UWORD flags);
    void    VoiceStop(int voice);
    int     VoiceActive(int voice);
    void    VoiceKeyOn(int voice);
    void    VoiceKeyOff(int voice);
    
    UBYTE   GetBPM();
    void    SetBPM(UBYTE bpm);

    MDRIVER_MXX();
    virtual ~MDRIVER_MXX();
};

#endif
