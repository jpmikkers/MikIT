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

