/*

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