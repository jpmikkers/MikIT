/*

File:           MDRV_RAW.H
Description:    -
Version:        1.00 - original

*/
#ifndef MDRV_RAW_H
#define MDRV_RAW_H

#include "mtypes.h"
#include "mdrv_fmx.h"
//#include "mikitdll.h"

class MDRIVER_RAW : public MDRIVER_FMX {

private:
    bool        isplaying;

public:
    void    Init();
    void    Exit();
    void    Start();
    void    Stop();
    void    Update();
    void    Pause();
    void    Resume();
    void    Break();

    ULONG   GetActualPosition();
    void    DriverFunc(int,void *);

    MDRIVER_RAW();
    virtual ~MDRIVER_RAW();

    static int GetDeviceName(char *name,int len,int devno);
};

#endif
