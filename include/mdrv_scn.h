/*

File:           MDRV_SCN.H
Description:    -
Version:        1.00 - original

*/
#ifndef MDRV_SCN_H
#define MDRV_SCN_H

#include "mtypes.h"
#include "minput.h"
#include "mdriver.h"

typedef float real;

#define MAXCHN 128
#define MAXSMP 256
#define TICKBUFFERSIZE  8192

#define FRACBITS 14
#define FRACMASK ((1L<<FRACBITS)-1)

#ifndef min
#define min(a,b) (((a)<(b)) ? (a) : (b))
#endif

class MDRIVER_SCN : public MDRIVER {

private:
    int     samples2bytes(int c);
    int     bytes2samples(int c); 
    int     tickleft;
    ULONG   mixposition;
    UBYTE   bpm;
    SLONG   samplesthatfit;

protected:  
    void    WriteSamples(SBYTE *buffer,int todo);
    int     WriteBytes(SBYTE *buffer,int todo);
    void    WriteSilentSamples(SBYTE *buffer,int todo);
    int     WriteSilentBytes(SBYTE *buffer,int todo);

public:
    virtual void Init();
    virtual void Exit();
    virtual void Start();
    virtual void Stop();
    virtual void Update();

    int     AllocVoice();
    void    FreeVoice(int v);

    virtual int     PrepareSample(MSAMPLE *s);
    virtual void    UnPrepareSample(int handle);
//  virtual int     SampleLoad(MINPUT *i,ULONG length,ULONG reppos,ULONG repend,UWORD flags);
//    virtual void  SampleFree(int handle);

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

    MDRIVER_SCN();
    virtual ~MDRIVER_SCN();

    real    maxlevel;
    ULONG   GetMixPosition();
};

#endif
