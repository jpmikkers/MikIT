/*

File:                   MDRV_MDS.H
Description:    -
Version:        1.00 - original

*/
#ifndef MDRVDSK_H
#define MDRVDSK_H

#include <stdio.h>
#include "mtypes.h"
#include "mdrv_fmx.h"


class MDRIVER_MIDAS : public MDRIVER_FMX {

private:

        unsigned char *buffer;
        int     streammode;
        int     buffersize;
        int     bytestofill;
        int     readindex;
        int     writeindex;

public:
    void    Init();
        void    Exit();
        void    Start();
        void    Stop();
    void    Update();

        MDRIVER_MIDAS();
        virtual ~MDRIVER_MIDAS();

        void   *midashandle;
};

#endif
