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
