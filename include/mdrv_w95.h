/*

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