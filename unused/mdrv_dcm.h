/*

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