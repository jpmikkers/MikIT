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

File:           MMODULE.CPP
Description:    -
Version:        1.00 - original

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mmodule.h"
#include "mdrv_scn.h"


MMODULE::MMODULE()
{
    loopmode=1;
    scanmode=0;
    duration=0;
    terminator=NULL;
    terminatordata=NULL;
    infomode=false;
}


MMODULE::~MMODULE()
{
}



void MMODULE::SetLoopMode(int yesno)
{
    loopmode=yesno;
}


int MMODULE::GetLoopMode()
{
    return loopmode;
}


void MMODULE::SetScanMode(int yesno)
{
    scanmode=yesno;
}


bool MMODULE::Terminated()
{
    if(terminator!=NULL){
        return terminator(terminatordata);
    }
    return false;
}


void MMODULE::TerminateThrow()
{
    if(Terminated()) THROW MikModException("Terminated");
}


int MMODULE::GetScanMode()
{
    return scanmode;
}



int MMODULE::GetSampleName(char *d,int l,int i)
{
    int num=GetNumSamples();
    
    if(i<0)     return num;
    if(i>=num)  return 0;

    if(l==0) return strlen(GetSampleName(i));
    strncpy(d,GetSampleName(i),l);
    
    return 1;
}



int MMODULE::GetInstrumentName(char *d,int l,int i)
{
    int num=GetNumInstruments();

    if(i<0)    return num;
    if(i>=num) return 0;

    if(l==0)   return strlen(GetInstrumentName(i));
    strncpy(d,GetInstrumentName(i),l);
    return 1;
}


int MMODULE::GetSongTitle(char *d,int l)
{
    int t;
    char *s=GetSongTitle();

    if(l==0) return strlen(s);
    
    if(l>(int)strlen(s)) l=strlen(s);
    
    for(t=0;t<l;t++){
        if(s[t]==0 || s[t]=='\n') break;
        d[t]=s[t];
    }
    d[t]=0;
    return 1;
}


void MMODULE::BuildEventList()
{
    MDRIVER_SCN d;

    d.mode=DMODE_16BITS|DMODE_STEREO|DMODE_SCAN;
    d.channels=2;
    d.frequency=8000;
    d.latency=100;
    d.tickhandler=NULL;
    d.uservolume=100;
    d.Init();
    SetLoopMode(0);
    SetScanMode(1);
    Start(&d);
    d.Start();

    duration=0;
    while(!IsReady() && GetTimeStamp()<(30*60*1000)) Update();
    duration=GetTimeStamp();

    d.Stop();
    Stop();
    d.Exit();
    SetLoopMode(1);
    SetScanMode(0);
}


void MMODULE::Seek(int time)
{
    if(driver==NULL ) return;
    if(time>duration) return;

    driver->Pause();

    if(GetTimeStamp()>time) Restart();
    
    scanmode=1;
    while(GetTimeStamp()<time && !IsReady()) Update();
    scanmode=0;

    driver->Resume();
}


int MMODULE::GetDuration()
{
    return duration;
}


void MMODULE::SetTerminator(bool (*f)(void *),void *d)
{
    terminator=f;
    terminatordata=d;
}
