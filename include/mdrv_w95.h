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

File:           MDRV_W95.H
Description:    -
Version:        1.00 - original

*/
#ifndef MDRV_W95_H
#define MDRV_W95_H

#include "mtypes.h"
#include "mdrv_fmx.h"
#include <windows.h>


#define NUMBUFFERS 4


class MDRIVER_W95 : public MDRIVER_FMX {

private:

    MDRIVER_W95 *parentthis;
    DWORD  parentthread;
    unsigned mixerthread;
    HANDLE mixerthreadhandle;

    HANDLE eventhandle;

    enum{
        WM_MIXER_OK = WM_USER+1,
        WM_MIXER_DIE,
        WM_MIXER_BREAK,
        WM_MIXER_PAUSE,
        WM_MIXER_RESUME,
        WM_MIXER_START
    };

    WAVEHDR     WaveOutHdr[NUMBUFFERS];
    WAVEOUTCAPS woc;
    HWAVEOUT    hWaveOut;
    boolean     isplaying;
    int         filled;
    int         nextone;
    int         buffersize;
//  ULONG       mixposition;
    UBYTE      *bigbuffer;

    void    KillMixer();
    void    KillAudio();

    static unsigned _stdcall StaticMixerThread(void *p);
    void LocalMixerThread();
    int     SpecialUpdate();

    void    PrivateGetScopeData(SBYTE *ldata,SBYTE *rdata,int todo);

public:
    int     priority;

    void    Init();
    void    Exit();
    void    Start();
    void    Stop();
    void    Update();
    void    Pause();
    void    Resume();
    void    Break();

    ULONG   GetActualPosition();

    void    GetScopeData(SBYTE data[2][576]);
//  void    GetSpectrumData(SBYTE data[2][576]);
    ULONG   PrivateGetSynchPosition();

    MDRIVER_W95();
    virtual ~MDRIVER_W95();

    static int GetDeviceName(char *name,int len,int devno);
};

#endif