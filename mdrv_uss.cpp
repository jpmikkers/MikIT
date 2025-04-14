/*
  Linux sound driver
*/
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include "mdrv_uss.h"

#define DEFAULT_FRAGSIZE 17
#define DEFAULT_NUMFRAGS 4

MDRIVER_USS::MDRIVER_USS() : MDRIVER_FMX()
{
    sndfd=-1;
    audiobuffer=NULL;
}


MDRIVER_USS::~MDRIVER_USS()
{
}


void MDRIVER_USS::Init()
{
    char *env;
    int play_precision,play_stereo,play_rate;
    int fragsize,numfrags;

    if((sndfd=open("/dev/dsp",O_WRONLY))<0)
        THROW MikModException("Cannot open sounddevice");

    fragsize=(env=getenv("MM_FRAGSIZE")) ? atoi(env) : DEFAULT_FRAGSIZE;
    numfrags=(env=getenv("MM_NUMFRAGS")) ? atoi(env) : DEFAULT_NUMFRAGS;

    if(fragsize<7 || fragsize>17)  fragsize=DEFAULT_FRAGSIZE;
    if(numfrags<2 || numfrags>255) numfrags=DEFAULT_NUMFRAGS;

    fragmentsize=(numfrags<<16) | fragsize;

    if(ioctl(sndfd, SNDCTL_DSP_SETFRAGMENT, &fragmentsize)<0)
        THROW MikModException("Buffer fragment failed");

    play_precision = (mode & DMODE_16BITS) ? 16 : 8;
    play_stereo= (mode & DMODE_STEREO) ? 1 : 0;
    play_rate=frequency;

    if(ioctl(sndfd, SNDCTL_DSP_SAMPLESIZE, &play_precision) == -1 ||
       ioctl(sndfd, SNDCTL_DSP_STEREO, &play_stereo) == -1 ||
       ioctl(sndfd, SNDCTL_DSP_SPEED, &play_rate) == -1)
        THROW MikModException("Device can't play sound in this format");

    ioctl(sndfd, SNDCTL_DSP_GETBLKSIZE, &fragmentsize);

    frequency=play_rate;    // v0.91 retrieve actual frequency now

/*  Lose this for now - it will confuse ncurses etc...
    printf("Fragment size is %ld\n",fragmentsize); */

    audiobuffer = (char*) malloc(fragmentsize * sizeof(char));

    if(audiobuffer==NULL)
        THROW MikModException("Couldn't allocate sound buffer");

    MDRIVER_FMX::Init();
}


void MDRIVER_USS::Start()
{
    MDRIVER_FMX::Start();
}

void MDRIVER_USS::Stop()
{
    MDRIVER_FMX::Stop();
}


void MDRIVER_USS::Exit()
{
    free(audiobuffer);
    close(sndfd);
    MDRIVER_FMX::Exit();
}


void MDRIVER_USS::Update()
{
    WriteBytes((signed char *)audiobuffer,fragmentsize);
    write(sndfd,audiobuffer,fragmentsize);
}
