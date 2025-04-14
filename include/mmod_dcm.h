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
#ifndef MMODDCM_H
#define MMODDCM_H

#include "mtypes.h"
#include "minput.h"
#include "mmodule.h"
#include "mdriver.h"

/****************************************************************************
 MMODULE_MOD class :
****************************************************************************/


class MMODULE_DCM : public MMODULE {

/************************* LOADER STUFF *************************/

private:

    class DCMSAMPLE{       /* sample header as it appears in a module */
	
    public:
		ULONG length;
		ULONG loopstart;
		ULONG loopend;
		UWORD flags;
	    UWORD id;

        // extra information:

        int  handle;
		MSAMPLE *msample;

		void LoadHeader(MINPUT *i);
		void LoadBody(MINPUT *i);

		DCMSAMPLE();
		~DCMSAMPLE();
    };
	
	UBYTE numchn;
	UBYTE numsmp;

	ULONG streamsize;
	ULONG streamrepp;

    // patterns
	UBYTE     *pc;
	UBYTE     *streamend;

    UBYTE      *pattern;
	DCMSAMPLE  *samples;

	int   voices[256];
	UBYTE instrument[256];
	UBYTE nopcount;

	UBYTE isready;

	int    timestamp;

public:
    static int Test(MINPUT *i,char *title,int maxlen);
    void Load(MINPUT *i);
	
    MMODULE_DCM();
    ~MMODULE_DCM();

/************************* PLAYER STUFF *************************/

private:
    int isplaying;

public:
    void Start(MDRIVER *d);
    void Stop();
    void Update();
	void Restart();

	int GetNumChannels();
	int GetSongLength();
	int GetPos();
	void NextPos();
	void PrevPos();
	int IsReady();
	char *GetSongTitle();
	char *GetSampleName(int s);
	int  GetNumSamples();

	int  GetTimeStamp();
	void SetTimeStamp(int ts);
};

#endif