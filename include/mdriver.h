/*

File:           MDRIVER.H
Description:    -
Version:        1.00 - original

*/
#ifndef MDRIVER_H
#define MDRIVER_H

#include "mtypes.h"
#include "minput.h"

/****************************************************************************
 DRIVER base class :
****************************************************************************/

#define MAXCHN 128

#define SF_16BITS       1
#define SF_SIGNED       2
#define SF_STEREO       4
#define SF_LOOP         8
#define SF_BIDI         16
#define SF_SUSLOOP      32
#define SF_SUSBIDI      64
#define SF_DELTA        128
#define SF_REVERSE      256
#define SF_BIGENDIAN    512
#define SF_FTPAN        1024
#define SF_ITCMP        2048
#define SF_SURROUND     4096
#define SF_ITCDL        8192

#define DMODE_16BITS    1
#define DMODE_STEREO    2
#define DMODE_INTERP    4
#define DMODE_NOCLICK   8
#define DMODE_SCAN      16
#define DMODE_DITHER    32

class MCONVERT {

private:
    SWORD buffer[1024];

    MINPUT *in;
    UWORD infmt;
    UWORD outfmt;
    SWORD oldsample;

    /* compression vars: */

    UWORD compressedblocksize;
    UBYTE bitdepth;
    UBYTE lastvalue;
    UBYTE loldvalue;
    UWORD lastword;
    UWORD loldword;
    UWORD shiftbuffer;
    UWORD shiftbits;
    UWORD bytesleft;

    void decomp8(SBYTE *buffer,int todo);
    void decomp16(SWORD *buffer,int todo);
    ULONG getbits(int bits);
    void readwords(SWORD *buffer,int todo);
    void readbytes(SBYTE *buffer,int todo);

public:

    void Init(MINPUT *i,UWORD infmt,UWORD outfmt);
    void ReadSamples(void *buf,ULONG count);
    void ReadBytes(void *buf,ULONG count);
};


class MSAMPLE
{
    private:
    ULONG length;
    ULONG reppos;
    ULONG repend;
    UWORD flags;

    public:
    void  *data;

    MSAMPLE(MINPUT *i,ULONG length,ULONG reppos,ULONG repend,UWORD flags);
    ~MSAMPLE();
};


class MDRIVER {

protected:
    MCONVERT converter;

    virtual void    Tick();

public:
    
    MDRIVER();
    virtual ~MDRIVER();

    // main parameters (have to be set BEFORE Init())

    void  (*tickhandler)(void *);   // tick handler
    void  *tickhandlerdata;         // tick handler data
    ULONG frequency;                // mixing frequency
    UWORD latency;                  // delay in milliseconds
    UWORD mode;                     // stereo, 16 bits
    UWORD channels;                 // number of channels requested by user
    UWORD uservolume;               // user volume
    int   subdevice;                // subdevice number
    float ampfactor;                // amplification factor

    // main routines

    virtual void    Init()              =0;
    virtual void    Exit()              =0;
    virtual void    Start()             =0;
    virtual void    Stop()              =0;
    virtual void    Update()            =0;

    virtual int     PrepareSample(MSAMPLE *s)=0;
    virtual void    UnPrepareSample(int handle)=0;

    virtual UBYTE   GetBPM()            =0;
    virtual void    SetBPM(UBYTE bpm)   =0;

    virtual int     AllocVoice()        =0;
    virtual void    FreeVoice(int v)    =0;

    virtual void    VoiceSetVolume(int v,UBYTE vol)=0;
    virtual void    VoiceSetPanning(int v,UBYTE pan)=0;
    virtual void    VoiceSetFrequency(int v,ULONG frq)=0;
    virtual void    VoiceSetFilter(int voice,UBYTE cutoff,UBYTE damping)=0;
    virtual void    VoicePlay(int voice,int handle,ULONG start,ULONG size,ULONG reppos,ULONG repend,ULONG sreppos,ULONG srepend,UWORD flags)=0;
    virtual void    VoiceStop(int voice)=0;
    virtual int     VoiceActive(int voice)=0;
    virtual void    VoiceKeyOn(int voice)=0;
    virtual void    VoiceKeyOff(int voice)=0;

    virtual ULONG   GetMixPosition()    { return 0; }
    virtual ULONG   GetActualPosition() { return 0; }

    virtual void    Pause()             { }
    virtual void    Resume()            { }
    virtual void    DriverFunc(int,void *)  { }
};

#endif
