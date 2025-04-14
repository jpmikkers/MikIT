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

File:           MDRV_DX5.H
Description:    -
Version:        1.00 - original

*/
#ifndef MDRV_DX5_H
#define MDRV_DX5_H

#include "dsound.h"
#include "mtypes.h"
#include "mdrv_fmx.h"
#include <windows.h>


class MDRIVER_DX5 : public MDRIVER_FMX {

private:
    MDRIVER_DX5 *parentthis;
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
    
    LPDIRECTSOUND ds;
    LPDIRECTSOUNDBUFFER db;
    ULONG bufferbytes;
    ULONG latencybytes;
    ULONG lastposition;
    bool  primary;

    ULONG lastw,lastc;

    void DS_Error(char *prefix,HRESULT result);

    void    KillMixer();
//  void    KillAudio();

    static unsigned _stdcall StaticMixerThread(void *p);
    void LocalMixerThread();

public:
    void    Init();
    void    Exit();
    void    Start();
    void    Stop();
    void    Update();
    void    Pause();
    void    Resume();
    void    Break();

    ULONG   GetMixPosition();
    ULONG   GetActualPosition();

    MDRIVER_DX5();
    virtual ~MDRIVER_DX5();

    static int GetDeviceName(char *name,int len,int devno);
};

#endif