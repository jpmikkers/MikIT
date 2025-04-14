/*

File:           MDRV_DSK.H
Description:    -
Version:        1.00 - original

*/
#ifndef MDRVDSK_H
#define MDRVDSK_H

#include <stdio.h>
#include "mtypes.h"
#include "mdrv_fmx.h"


class MDISK : public MDRIVER_FMX {

private:

    SBYTE   buffer[2048];
    FILE *dest;

public:
    void    Init();
    void    Exit();
    void    Start();
    void    Stop();
    void    Update();

    MDISK();
    virtual ~MDISK();
};

#endif