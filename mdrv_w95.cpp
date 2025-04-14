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
#include "mdrv_w95.h"


MDRIVER_W95::MDRIVER_W95() : MDRIVER_FMX()
{
    priority=0;
    isplaying=false;
    mixerthreadhandle=NULL;
    hWaveOut=NULL;
}


MDRIVER_W95::~MDRIVER_W95()
{
    KillMixer();
    KillAudio();
}


void MDRIVER_W95::KillMixer()
{
    if(mixerthreadhandle!=NULL){
        ResumeThread(mixerthreadhandle);
        while(!PostThreadMessage(mixerthread,WM_MIXER_DIE,0,0)) Sleep(20);
        WaitForSingleObject(mixerthreadhandle,INFINITE);
        mixerthreadhandle=NULL;
    }
}


void MDRIVER_W95::KillAudio()
{
    if(hWaveOut!=NULL){
        waveOutReset(hWaveOut);
        while(waveOutClose(hWaveOut)==WAVERR_STILLPLAYING) Sleep(20);
        hWaveOut=NULL;
    }
}


void MDRIVER_W95::Init()
{
    int t;
    WAVEFORMATEX wf;
    MMRESULT err;

    /* get audio device name and put it into the driver structure: */
    waveOutGetDevCaps((subdevice==0) ? WAVE_MAPPER : subdevice-1,&woc,sizeof(WAVEOUTCAPS));
    /* drv_w95.Name=woc.szPname; */

    if((mode & DMODE_STEREO) && (woc.wChannels<2) ){
        /* switch output mode to mono if device
            doesn't support stereo */
		mode&=~DMODE_STEREO;
	}

	if(mode & DMODE_16BITS){
		/* switch output mode to 8 bit if device
			doesn't support 16 bit output */
        if(!(woc.dwFormats &
             (WAVE_FORMAT_1M16 | WAVE_FORMAT_1S16 |
              WAVE_FORMAT_2M16 | WAVE_FORMAT_2S16 |
              WAVE_FORMAT_4M16 | WAVE_FORMAT_4S16))){
              mode&=~DMODE_16BITS;
        }
    }

    buffersize=(latency*frequency)/1000L;

    if(mode & DMODE_STEREO) buffersize<<=1;
    if(mode & DMODE_16BITS) buffersize<<=1;

    wf.wFormatTag=WAVE_FORMAT_PCM;
    wf.nChannels=(mode & DMODE_STEREO) ? 2 : 1;
    wf.nSamplesPerSec=frequency;
    wf.nAvgBytesPerSec=frequency;
    if(mode & DMODE_STEREO) wf.nAvgBytesPerSec<<=1;
    if(mode & DMODE_16BITS) wf.nAvgBytesPerSec<<=1;
    wf.nBlockAlign=1;
    if(mode & DMODE_STEREO) wf.nBlockAlign<<=1;
    if(mode & DMODE_16BITS) wf.nBlockAlign<<=1;
    wf.wBitsPerSample=(mode & DMODE_16BITS) ? 16 : 8;
    wf.cbSize=0;

    err=waveOutOpen(&hWaveOut,(subdevice==0) ? WAVE_MAPPER : subdevice-1,&wf,NULL,NULL,WAVE_FORMAT_QUERY|WAVE_ALLOWSYNC);

    if(err){
        if(err==WAVERR_BADFORMAT){
            THROW MikModException("This output format is not supported (Try another sampling rate?)");
        }
        else if(err==MMSYSERR_ALLOCATED){
            THROW MikModException("Audio device already in use");
        }
        else{
            THROW MikModException("Can't open audio device");
        }
    }

    bigbuffer=(UBYTE *)GlobalAlloc(GMEM_FIXED,buffersize*NUMBUFFERS);
    if(bigbuffer==NULL) THROW MikModException("Globalalloc failed");

    for(t=0;t<NUMBUFFERS;t++){
        WaveOutHdr[t].lpData=(char *)&bigbuffer[t*buffersize];
        WaveOutHdr[t].dwBufferLength=buffersize;
        WaveOutHdr[t].dwFlags=0;
        WaveOutHdr[t].dwLoops=0;
        WaveOutHdr[t].dwUser=0;
    }

    MDRIVER_FMX::Init();
//  mixposition=0;
}


unsigned _stdcall MDRIVER_W95::StaticMixerThread(void *p)
{
    (*((MDRIVER_W95 **)p))->LocalMixerThread();
    return 0;
}


void MDRIVER_W95::LocalMixerThread()
{
    MSG msg;
    int result;
    int quit=0;
    int t;

    // inform parent that we started ok (and create the message queue)
    while(!PostThreadMessage(parentthread,WM_MIXER_OK,0,0)) Sleep(20);

    while(!quit){
        result=GetMessage(&msg,NULL,0,0);

        if(result==TRUE){
            switch(msg.message){

                case WM_QUIT:
                    quit=1;
                    break;

                case WM_MIXER_DIE:
                    quit=1;
                    break;

                case WM_MIXER_BREAK:
                    waveOutReset(hWaveOut);
                    for(t=0;t<NUMBUFFERS;t++) waveOutUnprepareHeader(hWaveOut,&WaveOutHdr[t],sizeof(WAVEHDR));
                    filled=0;
                    nextone=0;
                    // mixposition=GetActualPosition();
                    SpecialUpdate();
                    Sleep(1);
                    SpecialUpdate();
                    break;

                case WM_MIXER_PAUSE:
                    waveOutPause(hWaveOut);
                    break;

                case WM_MIXER_RESUME:
                    waveOutRestart(hWaveOut);
                    break;

                case WM_MIXER_START:
					while(filled<NUMBUFFERS) SpecialUpdate();
					break;

                case MM_WOM_DONE:
					waveOutUnprepareHeader(hWaveOut,(LPWAVEHDR)msg.lParam,sizeof(WAVEHDR));
					filled--;
					while(filled<NUMBUFFERS) SpecialUpdate();
                    break;
            }
        }
    }
}


void MDRIVER_W95::Start()
{
    MSG msg;
    MMRESULT err;
    WAVEFORMATEX wf;

    filled=0;
    nextone=0;
//  mixposition=0;
    MDRIVER_FMX::Start();

    parentthis=this;
    parentthread=GetCurrentThreadId();

    // create a suspended mixer thread

    mixerthreadhandle=
        (HANDLE) _beginthreadex( 
        NULL, 0, StaticMixerThread, &parentthis, CREATE_SUSPENDED, &mixerthread );

    if(mixerthreadhandle==NULL) THROW MikModException("Couldn't create mixer thread");

    wf.wFormatTag=WAVE_FORMAT_PCM;
    wf.nChannels=(mode & DMODE_STEREO) ? 2 : 1;
    wf.nSamplesPerSec=frequency;
    wf.nAvgBytesPerSec=frequency;
    if(mode & DMODE_STEREO) wf.nAvgBytesPerSec<<=1;
    if(mode & DMODE_16BITS) wf.nAvgBytesPerSec<<=1;
    wf.nBlockAlign=1;
    if(mode & DMODE_STEREO) wf.nBlockAlign<<=1;
    if(mode & DMODE_16BITS) wf.nBlockAlign<<=1;
    wf.wBitsPerSample=(mode & DMODE_16BITS) ? 16 : 8;
    wf.cbSize=0;

    err=waveOutOpen(&hWaveOut,(subdevice==0) ? WAVE_MAPPER : subdevice-1,&wf,mixerthread,NULL,WAVE_ALLOWSYNC|CALLBACK_THREAD);

    if(err){
        if(err==WAVERR_BADFORMAT){
            THROW MikModException("This output format is not supported (Try another sampling rate?)");
        }
        else if(err==MMSYSERR_ALLOCATED){
            THROW MikModException("Audio device already in use");
        }
        else{
            THROW MikModException("Can't open audio device");
        }
    }

    // kickstart the mixer thread
    ResumeThread(mixerthreadhandle);
    
    // wait for the mixer to become active
    GetMessage(&msg,NULL,WM_MIXER_OK,WM_MIXER_OK);

    // and tell it to start mixing
    while(!PostThreadMessage(mixerthread,WM_MIXER_START,0,0)) Sleep(20);

    isplaying=true;
}


void MDRIVER_W95::Stop()
{
    isplaying=false;

    KillMixer();

    for(int t=0;t<NUMBUFFERS;t++){
        waveOutReset(hWaveOut);
        waveOutUnprepareHeader(hWaveOut,&WaveOutHdr[t],sizeof(WAVEHDR));
    }

    KillAudio();
    
    MDRIVER_FMX::Stop();
}


void MDRIVER_W95::Exit()
{
    GlobalFree(bigbuffer);
    MDRIVER_FMX::Exit();
}


void MDRIVER_W95::Update()
{
}


void MDRIVER_W95::Break()
{
    PostThreadMessage(mixerthread,WM_MIXER_BREAK,0,0);
}


void MDRIVER_W95::Pause()
{
    PostThreadMessage(mixerthread,WM_MIXER_PAUSE,0,0);
}


void MDRIVER_W95::Resume()
{
    PostThreadMessage(mixerthread,WM_MIXER_RESUME,0,0);
}


int MDRIVER_W95::SpecialUpdate()
{
    WriteBytes((SBYTE *)WaveOutHdr[nextone].lpData,buffersize);
    WaveOutHdr[nextone].dwBufferLength=buffersize;
    WaveOutHdr[nextone].dwFlags=0;
    WaveOutHdr[nextone].dwLoops=0;
	WaveOutHdr[nextone].dwUser=0;
	//	mixposition+=buffersize;
    waveOutPrepareHeader(hWaveOut,&WaveOutHdr[nextone],sizeof(WAVEHDR));
	waveOutWrite(hWaveOut,&WaveOutHdr[nextone],sizeof(WAVEHDR));
    filled++;
	nextone=(nextone+1)%NUMBUFFERS;
    return 1;
}


/*ULONG MDRIVER_W95::GetMixPosition()
{
    return mixposition;
}
*/

ULONG MDRIVER_W95::GetActualPosition()
{
    MMTIME tt;
    tt.wType=TIME_SAMPLES;
    waveOutGetPosition(hWaveOut,&tt,sizeof(tt));
    return tt.u.sample;
}


void MDRIVER_W95::PrivateGetScopeData(SBYTE *ldata,SBYTE *rdata,int todo)
{
    if(!isplaying){
        memset(ldata,0,todo);
        memset(rdata,0,todo);
        return;
    }
    
    int left,i=0;
    int q,pos=GetActualPosition() % (NUMBUFFERS*buffersize);

    while(todo>0){

        left=((NUMBUFFERS*buffersize)-pos); 
        if(mode&DMODE_STEREO) left>>=1;
        if(mode&DMODE_16BITS) left>>=1;
        if(left>todo) left=todo;

        if(mode & DMODE_STEREO){
            if(mode & DMODE_16BITS){
                for(q=0;q<left;q++){
                    ldata[i]=*((SWORD *)&bigbuffer[pos])>>8;
                    rdata[i]=*((SWORD *)&bigbuffer[pos+2])>>8;
                    i++;
                    pos+=4;
                }
            }
            else{
                for(q=0;q<left;q++){
                    ldata[i]=128+bigbuffer[pos];
                    rdata[i]=128+bigbuffer[pos+1];
                    i++;
                    pos+=2;
                }
            }
        }
        else{
            if(mode & DMODE_16BITS){
                for(q=0;q<left;q++){
                    ldata[i]=*((SWORD *)&bigbuffer[pos])>>8;
                    rdata[i]=ldata[i];
                    i++;
                    pos+=2;
                }
            }
            else{
                for(q=0;q<left;q++){
                    ldata[i]=128+bigbuffer[pos];
                    rdata[i]=ldata[i];
                    i++;
                    pos++;
                }
            }
        }

        if(pos>=(NUMBUFFERS*buffersize)) pos=0;
        todo-=left;
    }
}


ULONG MDRIVER_W95::PrivateGetSynchPosition()
{
    ULONG result=GetActualPosition();

    if(mode&DMODE_STEREO) result>>=1;
    if(mode&DMODE_16BITS) result>>=1;

    return MulDiv(result,44100,frequency);
}


int MDRIVER_W95::GetDeviceName(char *s,int l,int d)
{
    WAVEOUTCAPS woc;
    int numdevs=waveOutGetNumDevs()+1;

    if(d<0)         return numdevs;
    if(d>=numdevs)  return 0;

    /* get audio device name and put it into the driver structure: */
    waveOutGetDevCaps((d==0) ? WAVE_MAPPER : (d-1),&woc,sizeof(WAVEOUTCAPS));

    if(s==NULL || l==0) return strlen((const char *)woc.szPname);

    strncpy(s,(const char *)woc.szPname,l);
    return 1;
}
