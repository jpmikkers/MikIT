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

File:           MMOD_S31.CPP
Description:    -
Version:        1.00 - original

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mmod_s3m.h"


MMODULE_S3M::S3MPATTERN::S3MPATTERN()
{
    data=NULL;
}


MMODULE_S3M::S3MPATTERN::~S3MPATTERN()
{
    delete[] data;
}


UBYTE *MMODULE_S3M::S3MPATTERN::FakeRow(UBYTE *p)
{
    UBYTE flag;
    while((flag=*p++)!=0){
        if(flag&32)     p+=2;
        if(flag&64)     p+=1;
        if(flag&128)    p+=2;
    }
    return p;
}


void MMODULE_S3M::S3MPATTERN::DecodeRow(int row,S3MNOTE notes[32])
{
    UBYTE flag;
    UBYTE channel;
    UBYTE *p;

    p=&data[index[row]];

    for(channel=0;channel<32;channel++){
        notes[channel].nte=255;
        notes[channel].ins=0;
        notes[channel].vol=255;
        notes[channel].eff=255;
        notes[channel].dat=0;
    }

    while((flag=*p++)!=0){

        channel=flag&31;

        if(flag&32){
            notes[channel].nte=*p++;
            notes[channel].ins=*p++;
        }

        if(flag&64){
            notes[channel].vol=*p++;
        }

        if(flag&128){
            notes[channel].eff=*p++;
            notes[channel].dat=*p++;
        }
    }
}


void MMODULE_S3M::S3MPATTERN::Fake()
{
    data=new UBYTE[64];
    memset(data,0,64);

    UBYTE *p=data;

    for(int t=0;t<64;t++){
        index[t]=p-data;
        p=FakeRow(p);
    }
}


void MMODULE_S3M::S3MPATTERN::Load(MINPUT *in,ULONG seekpos)
{
    ULONG length;

    if(seekpos==0){
        length=0;
    }
    else{
        in->seek(seekpos,SEEK_SET);
        length=in->read_UWORD();
    }

//  printf("%x\n",length);

    if(length>10560) THROW MikModException("Illegal pattern");

    if(length>0){
        data=new UBYTE[length];
        in->read_UBYTES(data,length);
    }
    else{
        data=new UBYTE[64];
        memset(data,0,64);
    }

    UBYTE *p=data;

    for(int t=0;t<64;t++){
        index[t]=p-data;
        p=FakeRow(p);
//      printf("%d - %d\n",t,index[t]);
    }
}


MMODULE_S3M::S3MSAMPLE::S3MSAMPLE()
{
    handle=-1;
    msample=NULL;
}


MMODULE_S3M::S3MSAMPLE::~S3MSAMPLE()
{
    delete msample;
}


void MMODULE_S3M::S3MSAMPLE::Load(MINPUT *i,ULONG seekpos)
{
    if(seekpos==0) THROW MikModException("Zero sample paraptr");

    i->seek(seekpos,SEEK_SET);

    UBYTE type=i->read_UBYTE();     // sample type
    i->seek(12,SEEK_CUR);           // skip filename
    
    ULONG spar=i->read_UBYTE();
    spar<<=16;
    spar|=i->read_UWORD();
    spar<<=4;

    length      =i->read_ULONG();
    loopstart   =i->read_ULONG();
    loopend     =i->read_ULONG();
    volume      =i->read_UBYTE();

    i->seek(2,SEEK_CUR);            // skip dsk and pck

    UBYTE flg   =i->read_UBYTE();   

//  flags=0;                        // is initialised by caller
    if(flg&1) flags|=SF_LOOP;
    if(flg&4) flags|=SF_16BITS;

    c2spd       =i->read_ULONG();

    i->seek(12,SEEK_CUR);           // skip 12 unused bytes
    i->read_STRING(name,28);        // read sample name
    
//  printf("%s\n",name);
    
    UBYTE id[4];
    i->read_UBYTES(id,4);           // read id

    // DON'T load sample if it doesn't have the SCRS tag
    if(memcmp(id,"SCRS",4)!=0) length=0;

    if(length){
        i->seek(spar,SEEK_SET);
        msample=new MSAMPLE(i,length,loopstart,loopend,flags);
        if(i->eof()) THROW MikModException("sample body corrupted");
    }
}

