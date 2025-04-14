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

File:           MMOD_MD1.CPP
Description:    -
Version:        1.00 - original

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mmod_mod.h"


MMODULE_MOD::MODPATTERN::MODPATTERN()
{
    data=NULL;  
}


MMODULE_MOD::MODPATTERN::~MODPATTERN()
{
    delete[] data;
}


void MMODULE_MOD::MODPATTERN::Load(MINPUT *in,UWORD numchn)
{
    MODNOTE *p;
    UBYTE tmp[4];
    UBYTE note;
    
    static UWORD periods[5*12]={
        1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,906,
        856,808,762,720,678,640,604,570,538,508,480,453,
        428,404,381,360,339,320,302,285,269,254,240,226,
        214,202,190,180,170,160,151,143,135,127,120,113,
        107,101,95,90,85,80,75,71,67,63,60,56
    };

    p=data=new MODNOTE[64*numchn];

    for(int r=0;r<64;r++){
        for(int c=0;c<numchn;c++){
            in->read_UBYTES(tmp,4);

            p->ins= (tmp[0] & 0x10) | (tmp[2] >> 4);
            p->eff= tmp[2]&0xf;
            p->dat= tmp[3];

            UWORD period=(((UWORD)tmp[0] & 0x0f) << 8) + tmp[1] ;

            for(note=0;note<60;note++){
                    if(period>=periods[note]){
                        break;
                    }
            }

            if(note==60){
                p->nte=0;
            }
            else{
                p->nte=note+1;
            }
    
            p++;
        }                           
    }

    if(in->eof()) THROW MikModException("EOF while reading pattern data");
}


MMODULE_MOD::MODSAMPLE::MODSAMPLE()
{
    handle=-1;
    msample=NULL;
}


MMODULE_MOD::MODSAMPLE::~MODSAMPLE()
{
    delete msample;
}


void MMODULE_MOD::MODSAMPLE::LoadHeader(MINPUT *i)
{
    i->read_STRING(name,22);            /* (char) Instrument name */

    length=i->read_UWORD();
    length<<=1;

    finetune=i->read_UBYTE();
    volume=i->read_UBYTE();

    loopstart=i->read_UWORD();
    loopstart<<=1;

    ULONG replen=i->read_UWORD();
    loopend=loopstart + (replen<<1);

    flags=0;

    flags|=SF_SIGNED;
    if(replen>1) flags|=SF_LOOP;

    /* fix replen if repend>length */
    if(loopend>length) loopend=length;
}


void MMODULE_MOD::MODSAMPLE::LoadBody(MINPUT *in)
{
    msample=new MSAMPLE(in,length,loopstart,loopend,flags);
    if(in->eof()) THROW MikModException("sample body corrupted");
}

