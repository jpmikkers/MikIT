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

File:           MDRV_WAV.CPP
Description:    -
Version:        1.00 - original
                1.01 - fixed lengths written to WAV header.. they were 4 bytes short
                1.02 - fixed lengths written to WAV header AGAIN @$^#!$#%$
*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <assert.h>
#include "mdrv_wav.h"



MDRV_WAV::MDRV_WAV(FILE *fp) : MDRIVER_FMX()
{
    dest=fp;
}



MDRV_WAV::~MDRV_WAV()
{
}



void MDRV_WAV::Init()
{
    MDRIVER_FMX::Init();
}



void MDRV_WAV::Exit()
{
    MDRIVER_FMX::Exit();
}



void MDRV_WAV::Start()
{
    ULONG ltmp;
    UWORD wtmp;

    fwrite("RIFF",4,1,dest);            // rID
    fwrite("XXXX",4,1,dest);            // rLen
    fwrite("WAVE",4,1,dest);            // wID
    fwrite("fmt ",4,1,dest);            // fID
    ltmp=16; fwrite(&ltmp,4,1,dest);    // fLen

    wtmp=0x0001;                        // WAVE_FORMAT_PCM;  
    fwrite(&wtmp,2,1,dest);             // wFormatTag

    wtmp=(mode & DMODE_STEREO) ? 2 : 1;
    fwrite(&wtmp,2,1,dest);             // nChannels

    ltmp=frequency;
    fwrite(&ltmp,4,1,dest);         // nSamplesPerSec

    if(mode & DMODE_STEREO) ltmp<<=1;
    if(mode & DMODE_16BITS) ltmp<<=1;
    fwrite(&ltmp,4,1,dest);         // nAvgBytesPerSec

    wtmp=1;
    if(mode & DMODE_16BITS) wtmp<<=1;
    if(mode & DMODE_STEREO) wtmp<<=1;
    fwrite(&wtmp,2,1,dest);         // nBlockAlign

    wtmp=(mode & DMODE_16BITS) ? 16 : 8;
    fwrite(&wtmp,2,1,dest);         // wBitsPerSample

    fwrite("data",4,1,dest);        // dID
    fwrite("XXXX",4,1,dest);        // dLen
    
    MDRIVER_FMX::Start();
}


void MDRV_WAV::Stop()
{
    ULONG len,ltmp;
    MDRIVER_FMX::Stop();
    fflush(dest);
    len=ftell(dest);

    ltmp=len-8;             
    fseek(dest,4,SEEK_SET);
    fwrite(&ltmp,4,1,dest);

    ltmp=len-44;
    fseek(dest,40,SEEK_SET);
    fwrite(&ltmp,4,1,dest);

    fseek(dest,0,SEEK_END);
}


void MDRV_WAV::Update()
{
    WriteBytes(buffer,4096);
    fwrite(buffer,4096,1,dest);
}
