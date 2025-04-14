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

File:           MDRV_DCM.H
Description:    -
Version:        1.00 - original

*/
#ifndef MDRVDCM_H
#define MDRVDCM_H

#include "mtypes.h"
#include "minput.h"
#include "mdrv_mix.h"
 
class MDCM : public MDRIVER_MIXER {

private:

    struct SI{
        ULONG length;
        ULONG loopstart;
        ULONG loopend;
        UWORD flags;
        UWORD id;
    };

    SI    dcm_si[MAXSMP];
    char *dcm_sp[MAXSMP];

    SBYTE   buffer[1024];
    
    VOICEPARMS oldchn[MAXCHN];      // array of voices plus one dummy voice
    UBYTE      oldbpm;
    
    VOICEPARMS newchn[MAXCHN];      // array of voices plus one dummy voice
    UBYTE      newbpm;

    UBYTE      lastflag;
    int        dcm_loop;
    int        dcm_start;

    ULONG      dcm_streambase;
    ULONG      dcm_restartpos;
    FILE      *dcm_fp;
    
    void       WriteDCM();
    UWORD      ConvertFrq(ULONG freq);

    char      *dcm_filename;

protected:
    virtual void Tick();

public:
    void    Init();
    void    Exit();
    void    Start();
    void    Stop();
    void    Update();

    int     SampleLoad(MINPUT *i,ULONG length,ULONG reppos,ULONG repend,UWORD flags);
    void    SampleFree(int handle);

    MDCM(char *fn,int chn);
    virtual ~MDCM();
};

#endif