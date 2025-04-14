/*

File:           MDRV_USS.H
Description:    -
Version:        1.00 - original

*/
#ifndef MDRV_USS_H
#define MDRV_USS_H

#include <stdio.h>
#include <stdlib.h>
#include "mtypes.h"
#include "mdriver.h"
#include "mdrv_fmx.h"

class MDRIVER_USS : public MDRIVER_FMX {

private:
    int sndfd;
    int fragmentsize;
    char *audiobuffer;

public:
    void    Init();
    void    Exit();
    void    Start();
    void    Stop();
    void    Update();

    MDRIVER_USS();
    virtual ~MDRIVER_USS();
};

#endif

