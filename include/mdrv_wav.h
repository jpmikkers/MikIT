/*

(c) Copyright 1996, 1997 Jean-Paul Mikkers 

This file is part of MikIT.

MikIT is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 of 
the License, or (at your option) any later version.

MikIT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public 
License along with MikIT. If not, see <http://www.gnu.org/licenses/>.

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
