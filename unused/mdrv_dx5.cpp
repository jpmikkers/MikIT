/*

File:           MDRV_W95.CPP
Description:    -
Version:        1.00 - original
                1.01 - improved this thread bizniz
*/
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <windows.h>
#include <process.h>
#include "dsound.h"
#include "mdrv_dx5.h"

/*
HWND GetConsoleHwnd(void)
{
    #define MY_BUFSIZE 1024 // buffer size for console window titles
    HWND hwndFound;         // this is what is returned to the caller
    char pszNewWindowTitle[MY_BUFSIZE]; // contains fabricated WindowTitle
    char pszOldWindowTitle[MY_BUFSIZE]; // contains original WindowTitle
 
    // fetch current window title
 
    GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE);
 
    // format a "unique" NewWindowTitle
 
    wsprintf(pszNewWindowTitle,"%d/%d",
                GetTickCount(),
                GetCurrentProcessId());
 
    // change current window title
 
    SetConsoleTitle(pszNewWindowTitle);
 
    // ensure window title has been updated
 
    Sleep(40);
 
    // look for NewWindowTitle
 
    hwndFound=FindWindow(NULL, pszNewWindowTitle);
 
    // restore original window title
 
    SetConsoleTitle(pszOldWindowTitle);
 
    return(hwndFound);
}
*/

MDRIVER_DX5::MDRIVER_DX5() : MDRIVER_FMX()
{
    ds=NULL;
    db=NULL;
    primary=true;
//  primary=false;
    mixerthreadhandle=NULL;
}


void MDRIVER_DX5::KillMixer()
{
    if(mixerthreadhandle!=NULL){
        ResumeThread(mixerthreadhandle);
        while(!PostThreadMessage(mixerthread,WM_MIXER_DIE,0,0)) Sleep(20);
        WaitForSingleObject(mixerthreadhandle,INFINITE);
        mixerthreadhandle=NULL;
    }
}


MDRIVER_DX5::~MDRIVER_DX5()
{
    KillMixer();

    if(ds!=NULL){
        if(db!=NULL){
            ds->Release();
            ds=NULL;
        }
        ds->Release();
        ds=NULL;
    }
}


void MDRIVER_DX5::DS_Error(char *prefix,HRESULT result)
{
    static char errmsg[256];
    char *postfix;

    switch(result){
        case DSERR_ALLOCATED:
            postfix="resource already in use by another caller";
            break;

        case DSERR_CONTROLUNAVAIL:
            postfix="requested control unavailable";
            break;

        case DSERR_INVALIDPARAM:
            postfix="invalid parameter passed";
            break;

        case DSERR_INVALIDCALL:
            postfix="invalid call";
            break;

        case DSERR_GENERIC:
            postfix="undetermined error";
            break;

        case DSERR_PRIOLEVELNEEDED:
            postfix="caller needs higher priority level";
            break;

        case DSERR_OUTOFMEMORY:
            postfix="out of memory";
            break;

        case DSERR_BADFORMAT:
            postfix="specified WAVE format not supported";
            break;

        case DSERR_UNSUPPORTED:
            postfix="unsupported function called";
            break;

        case DSERR_NODRIVER:
            postfix="no sound driver available for use";
            break;

        case DSERR_ALREADYINITIALIZED:
            postfix="object already initialized";
            break;

        case DSERR_NOAGGREGATION:
            postfix="object does not support aggregation";
            break;

        case DSERR_BUFFERLOST:
            postfix="buffer lost";
            break;

        case DSERR_OTHERAPPHASPRIO:
            postfix="other app has priority";
            break;

        case DSERR_UNINITIALIZED:
            postfix="object not initialized";
            break;

        default:
            postfix="unknown directsound error";
            break;
    }

    sprintf(errmsg,"%s : %s",prefix,postfix);
    THROW MikModException(errmsg);
}


static BOOL CALLBACK DSEnumProc( LPGUID lpGUID, LPSTR lpszDesc,
                                LPSTR lpszDrvName, LPVOID lpContext )
{
        if(lpContext!=NULL) (*((int *)lpContext))++;
//    if(lpGUID!=NULL) memcpy( &guid, lpGUID, sizeof(GUID));
        return TRUE;
}


void MDRIVER_DX5::Init()
{
    HRESULT hr;
    DSCAPS  caps;
    DSBUFFERDESC dsbdesc;
    PCMWAVEFORMAT pcmwf;
    
    {
        int counter=0;
        DirectSoundEnumerate( (LPDSENUMCALLBACK)DSEnumProc,&counter);
        if(counter==0) THROW MikModException("No directsound drivers present");
    }

//  puts("directsound IS present");

    // try to open directsound object

    if((hr=DirectSoundCreate(0,&ds,NULL)) != DS_OK) DS_Error("Failed opening directsound object",hr);

//  puts("opened ok");

    // Create succeeded!, now set cooperative level 
    HWND hWnd = GetForegroundWindow();  if(hWnd == NULL) hWnd = GetDesktopWindow();

    if(primary){
        if((hr=ds->SetCooperativeLevel(hWnd/* GetConsoleHwnd() */,DSSCL_WRITEPRIMARY))!=DS_OK) DS_Error("Failed setting primary level",hr);
    }
    else{
        if((hr=ds->SetCooperativeLevel(hWnd,DSSCL_NORMAL))!=DS_OK) DS_Error("Failed setting normal level",hr);
    }

    // get device capabilities
    // Set up dscaps structure.
    memset(&caps, 0, sizeof(DSCAPS));
    caps.dwSize=sizeof(DSCAPS);

    if((hr=ds->GetCaps(&caps)) != DS_OK) DS_Error("Couldn't retrieve device capabilities",hr);

//  printf("Min %ld Max %ld\n",caps.dwMinSecondarySampleRate,caps.dwMaxSecondarySampleRate);

    // rescale mixing format to device capabilities
    if(primary){
        if(!(caps.dwFlags & DSCAPS_PRIMARY16BIT))  mode &= ~DMODE_16BITS;
        if(!(caps.dwFlags & DSCAPS_PRIMARYSTEREO)) mode &= ~DMODE_STEREO;
    }
    else{
        if(!(caps.dwFlags & DSCAPS_SECONDARY16BIT))  mode &= ~DMODE_16BITS;
        if(!(caps.dwFlags & DSCAPS_SECONDARYSTEREO)) mode &= ~DMODE_STEREO;
    }

    // Set up wave format structure.
    memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
    pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
    pcmwf.wf.nChannels = (mode & DMODE_STEREO) ? 2 : 1;
    pcmwf.wf.nSamplesPerSec = frequency;
    pcmwf.wf.nBlockAlign = 1;
    if(mode & DMODE_STEREO) pcmwf.wf.nBlockAlign <<= 1;
    if(mode & DMODE_16BITS) pcmwf.wf.nBlockAlign <<= 1;
    pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
    pcmwf.wBitsPerSample = (mode & DMODE_16BITS) ? 16 : 8;

    bufferbytes=(latency*frequency) / 1000;     
    if(mode & DMODE_STEREO) bufferbytes<<=1;
    if(mode & DMODE_16BITS) bufferbytes<<=1;
    latencybytes=bufferbytes;

    // Set up DSBUFFERDESC structure.

    if(primary){
        memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out.
        dsbdesc.dwSize = sizeof(DSBUFFERDESC);
        dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_STICKYFOCUS;      // DSBCAPS_PRIMARYBUFFER;   // | DSBCAPS_STICKYFOCUS;
        dsbdesc.dwBufferBytes = 0;
        dsbdesc.lpwfxFormat = NULL;             // Must be NULL for primary buffers.
    }
    else{
        memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out.
        dsbdesc.dwSize = sizeof(DSBUFFERDESC);
        dsbdesc.dwFlags = DSBCAPS_CTRLDEFAULT | DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE;      // DSBCAPS_PRIMARYBUFFER;   // | DSBCAPS_STICKYFOCUS;
        dsbdesc.dwBufferBytes = bufferbytes;
        dsbdesc.lpwfxFormat = (WAVEFORMATEX *)&pcmwf.wf;        // Must be NULL for primary buffers.
    }
    
    // Succeeded! Try to create buffer.
    if((hr=ds->CreateSoundBuffer(&dsbdesc,&db,NULL))!=DS_OK) DS_Error("Failed sound buffer",hr);

    if(primary){
        DSBCAPS dsbcaps;

    //    // Set primary buffer to desired format.    
        if((hr=db->SetFormat((LPWAVEFORMATEX)&pcmwf))!=DS_OK) DS_Error("Failed setting primary buffer format",hr);

        // succeeded.. get buffer capabilities
        memset(&dsbcaps,0,sizeof(DSBCAPS)); // Zero it out.
        dsbcaps.dwSize = sizeof(DSBCAPS);
    
        if((hr=db->GetCaps(&dsbcaps))!=DS_OK) DS_Error("Failed retrieving primary buffer size",hr);

        bufferbytes=dsbcaps.dwBufferBytes;
    }

    MDRIVER_FMX::Init();
//  mixposition=0;
}


void MDRIVER_DX5::Start()
{
    MDRIVER_FMX::Start();

//  SBYTE *p1,*p2;
//    ULONG c1,c2;

    lastw=latencybytes;
    lastc=0;

/*    if(db->Lock(0,lastw,(void **)&p1,&c1,(void **)&p2,&c2,0)==DS_OK){
        if(c1) WriteBytes(p1,c1);
        if(c2) WriteBytes(p2,c2);
        db->Unlock(p1,c1,p2,c2);
    }

*/  db->Play(0,0,DSBPLAY_LOOPING);
    db->SetVolume(0);
    lastposition=0;
//  isplaying=true;


    MSG msg;
//  MMRESULT err;
//  WAVEFORMATEX wf;

//  mixposition=0;

    parentthis=this;
    parentthread=GetCurrentThreadId();

    // create a suspended mixer thread

    mixerthreadhandle=
        (HANDLE) _beginthreadex( 
        NULL, 0, StaticMixerThread, &parentthis, CREATE_SUSPENDED, &mixerthread );

    if(mixerthreadhandle==NULL) THROW MikModException("Couldn't create mixer thread");

    // set thread priority

//  switch(priority){
//      case 1:
//          SetThreadPriority(mixerthreadhandle,THREAD_PRIORITY_ABOVE_NORMAL);
//          break;
//      case 2:
//          SetThreadPriority(mixerthreadhandle,THREAD_PRIORITY_HIGHEST);
//          break;
//      case 3:
            SetThreadPriority(mixerthreadhandle,THREAD_PRIORITY_TIME_CRITICAL);
//          break;
//      default:
//          SetThreadPriority(mixerthreadhandle,THREAD_PRIORITY_NORMAL);
//          break;
//  }
    
    // kickstart the mixer thread
    ResumeThread(mixerthreadhandle);
    
    // wait for the mixer to become active
    GetMessage(&msg,NULL,WM_MIXER_OK,WM_MIXER_OK);

    // and tell it to start mixing
    while(!PostThreadMessage(mixerthread,WM_MIXER_START,0,0)) Sleep(20);

//  isplaying=true;
}


void MDRIVER_DX5::Stop()
{
//  isplaying=false;
    KillMixer();
    db->Stop();
    MDRIVER_FMX::Stop();
}


void MDRIVER_DX5::Exit()
{
    MDRIVER_FMX::Exit();
    if(ds!=NULL){
        if(db!=NULL){
            db->Release();
            db=NULL;
        }
        ds->Release();
        ds=NULL;
    }
}

/*
void MDRIVER_DX5::Update()
{
    SBYTE *p1,*p2;
    ULONG c1,c2;
    ULONG todo;
    DWORD status;
    ULONG u1,u2;

    db->GetStatus(&status);

    if(status&DSBSTATUS_BUFFERLOST){
        if(db->Restore()!=DS_OK) return;
    }       

    db->GetStatus(&status);

    if(!(status&DSBSTATUS_PLAYING)){
        if(db->Play(0,0,DSBPLAY_LOOPING)!=DS_OK) return;
    }

    db->GetCurrentPosition(&u1,&u2);
    printf("\r%d   %d  %d    ",status,u1,u2);

    writepos=u2+latencybytes;
    if(writepos>=bufferbytes) writepos-=bufferbytes;

    if(u1==u2 || u1==lastposition) return;

    if(u1>lastposition){
        todo=u1-lastposition;
    }
    else{
        todo=(bufferbytes-lastposition)+u1;
    }

    todo&=0xfffffff0;

    if(todo && db->Lock(lastposition,todo,(void **)&p1,&c1,(void **)&p2,&c2,0)==DS_OK){
        if(p1 && c1) WriteBytes(p1,c1);
        if(p2 && c2) WriteBytes(p2,c2);
        db->Unlock(p1,c1,p2,c2);
    }

    lastposition+=todo;
    if(lastposition>=bufferbytes) lastposition-=bufferbytes;
}
*/


void MDRIVER_DX5::Update()
{
    SBYTE *p1,*p2;
    ULONG c1,c2;
    ULONG u1,u2;
    ULONG todo;
    DWORD status;

    db->GetStatus(&status);

    if(status&DSBSTATUS_BUFFERLOST){
        if(db->Restore()!=DS_OK) return;
    }       

    db->GetStatus(&status);

    if(!(status&DSBSTATUS_PLAYING)){
        if(db->Play(0,0,DSBPLAY_LOOPING)!=DS_OK) return;
    }

    db->GetCurrentPosition(&u1,&u2);
//    printf("\r%d   %d  %d    ",status,u1,u2);

    if(u1==u2 || u2==lastc) return;

    if(u2>=lastc){
        todo=u2-lastc;
    }
    else{
        todo=u2+(bufferbytes-lastc);
    }

    todo&=0xfffffffc;

    lastc=u2;

    if(todo<=latencybytes && db->Lock(lastw,todo,(void **)&p1,&c1,(void **)&p2,&c2,0)==DS_OK){
        if(p1 && c1) WriteBytes(p1,c1);
        if(p2 && c2) WriteBytes(p2,c2);
        db->Unlock(p1,c1,p2,c2);
    }

    lastw+=todo;
    if(lastw>=bufferbytes) lastw-=bufferbytes;
}


void MDRIVER_DX5::Break()
{
}


unsigned _stdcall MDRIVER_DX5::StaticMixerThread(void *p)
{
    (*((MDRIVER_DX5 **)p))->LocalMixerThread();
    return 0;
}


void MDRIVER_DX5::LocalMixerThread()
{
    MSG msg;
    int result;
    int quit=0;
//  int t;

    // inform parent that we started ok (and create the message queue)
    while(!PostThreadMessage(parentthread,WM_MIXER_OK,0,0)) Sleep(20);

    while(!quit){
//      result=GetMessage(&msg,NULL,0,0);
        result=PeekMessage(&msg,NULL,0,0,PM_REMOVE);

        if(result!=0){
            switch(msg.message){

                case WM_QUIT:
                    quit=1;
                    break;

                case WM_MIXER_DIE:
                    quit=1;
                    break;

                case WM_MIXER_BREAK:
/*                  waveOutReset(hWaveOut);
                    for(t=0;t<NUMBUFFERS;t++) waveOutUnprepareHeader(hWaveOut,&WaveOutHdr[t],sizeof(WAVEHDR));
                    filled=0;
                    nextone=0;
                    mixposition=GetActualPosition();
                    SpecialUpdate();
                    Sleep(1);
                    SpecialUpdate();
*/                  break;

                case WM_MIXER_PAUSE:
//                  waveOutPause(hWaveOut);
                    break;

                case WM_MIXER_RESUME:
//                  waveOutRestart(hWaveOut);
                    break;

                case WM_MIXER_START:
                case MM_WOM_DONE:
/*                  SpecialUpdate();
                    Sleep(1);
                    SpecialUpdate();
*/                  break;
            }
        }
        else{
            Update();
            Sleep(10);
        }
    }
}


void MDRIVER_DX5::Pause()
{
}


void MDRIVER_DX5::Resume()
{
}


ULONG MDRIVER_DX5::GetMixPosition()
{
    return 0;
//  return mixposition;
}


ULONG MDRIVER_DX5::GetActualPosition()
{
/*  MMTIME tt;
    tt.wType=TIME_BYTES;
    waveOutGetPosition(hWaveOut,&tt,sizeof(tt));
    return tt.u.cb;
*/
    return 0;
}


int MDRIVER_DX5::GetDeviceName(char *s,int l,int d)
{
//  WAVEOUTCAPS woc;
//  int numdevs=waveOutGetNumDevs()+1;

    if(d<0)         return 1;
//  if(d>=numdevs)  return 0;

    if(s==NULL || l==0) return strlen("directsound driver");

    strncpy(s,"directsound driver",l);
    return 1;
}

