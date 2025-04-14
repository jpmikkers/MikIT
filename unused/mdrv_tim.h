/*

File:           MDRV_USS.H
Description:    -
Version:        1.00 - original

*/
#ifndef MDRV_TIM_H
#define MDRV_TIM_H

#include <stdio.h>
#include <stdlib.h>
#include "mtypes.h"
#include "mdriver.h"
#include "mdrv_mxx.h"

class MDRIVER_TIM : public MDRIVER_MXX {

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

    MDRIVER_TIM();
    virtual ~MDRIVER_TIM();
};

#endif

