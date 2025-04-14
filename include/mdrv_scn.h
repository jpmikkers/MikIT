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
