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

File:           MMODULE.H
Description:    -
Version:        1.00 - original

*/
#ifndef MMODULE_H
#define MMODULE_H

#include <stdio.h>
#include <malloc.h>
#include "mtypes.h"
#include "minput.h"
#include "mdriver.h"

/****************************************************************************
 MODULE base class :
****************************************************************************/

#define MAXMODULEEVENTS 500

class MMODULE {

private:
        int  duration;

        bool (*terminator)(void *);
        void *terminatordata;

protected:
        int     loopmode;
        int     scanmode;
    
        MINPUT *in;
        MDRIVER *driver;
        
        bool Terminated();
        void TerminateThrow();
    
public:
        bool infomode;
        char *moduletype;
        
        // main routines

        void SetLoopMode(int yesno);
        int  GetLoopMode();

        void SetScanMode(int yesno);
        int  GetScanMode();

        virtual void Load(MINPUT *i)    {};
        virtual void Start(MDRIVER *d)  {};
        virtual void Stop()             {};
        virtual void Update()           {};
        virtual void Restart()          {};
        virtual int  GetSongLength()    { return 0; };
        virtual int  GetPos()           { return 0; };
        virtual int  GetRow()           { return 0; };
        virtual void NextPos()          {};
        virtual void PrevPos()          {};
        virtual int  IsReady()          { return 1; };

        virtual int  GetTimeStamp()     { return 0; };
        virtual void SetTimeStamp(int t){};

        virtual char *GetSongTitle()            { return NULL; }
        virtual char *GetSampleName(int n)      { return NULL; }
        virtual char *GetInstrumentName(int n)  { return NULL; }

        virtual int  GetNumChannels()       { return 0; };
        virtual int  GetNumSamples()        { return 0; };
        virtual int  GetNumInstruments()    { return 0; };

        virtual int  GetSongMessage(char *p,int maxlen) { return 0; };

        int  GetSongTitle(char *p,int maxlen);
        int  GetSampleName(char *d,int l,int i);
        int  GetInstrumentName(char *d,int l,int i);
        void BuildEventList();
        void Seek(int time);
        int  GetDuration();

        void SetTerminator(bool (*terminator)(void *),void *data);

        MMODULE();
        virtual ~MMODULE();
};


#endif
