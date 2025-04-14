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

File:           MDRV_RAW.CPP
Description:    -
Version:        1.00 - original
                1.01 - improved this thread bizniz
*/
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <windows.h>
#include <process.h>
#include "mdrv_raw.h"


MDRIVER_RAW::MDRIVER_RAW() : MDRIVER_FMX()
{
    isplaying=false;
}


MDRIVER_RAW::~MDRIVER_RAW()
{
}


void MDRIVER_RAW::Init()
{
    MDRIVER_FMX::Init();
}


void MDRIVER_RAW::Start()
{
    MDRIVER_FMX::Start();
    isplaying=true;
}


void MDRIVER_RAW::Stop()
{
    isplaying=false;
    MDRIVER_FMX::Stop();
}


void MDRIVER_RAW::Exit()
{
    MDRIVER_FMX::Exit();
}


void MDRIVER_RAW::Update()
{
}


void MDRIVER_RAW::Break()
{
}


void MDRIVER_RAW::Pause()
{
}


void MDRIVER_RAW::Resume()
{
}


void MDRIVER_RAW::DriverFunc(int funcno,void *funcdata)
{
    switch(funcno)
    {
        case MIKIT_DRIVERFUNC01:
            {
                MIKIT_DRIVERFUNC01DATA *d=(MIKIT_DRIVERFUNC01DATA *)funcdata;
                WriteBytes((SBYTE *)d->buffer,d->length);
            }
            break;

        default:
            THROW MikModException("function not supported by this device");
            break;
    }
}


ULONG MDRIVER_RAW::GetActualPosition()
{
    return GetMixPosition();
}


int MDRIVER_RAW::GetDeviceName(char *s,int l,int d)
{
    if(d<0) return 1;
    if(d>1) return 0;
    if(s==NULL || l==0) return strlen("Raw output");
    strncpy(s,"Raw output",l);
    return 1;
}
