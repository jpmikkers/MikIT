/*

*/
#ifndef MDRVWAV_H
#define MDRVWAV_H

#include <stdio.h>
#include "mtypes.h"
#include "mdrv_fmx.h"

class MDRV_WAV : public MDRIVER_FMX {

private:

    SBYTE   buffer[4096];
    FILE *dest;

public:
    void    Init();
    void    Exit();
        void    Start();
        void    Stop();
    void    Update();

    MDRV_WAV(FILE *fp);
    virtual ~MDRV_WAV();
};

#endif
