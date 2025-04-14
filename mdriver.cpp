/*

File:           MDRIVER.CPP
Description:    -
Version:        1.00 - original
                1.01 - v0.91 added IT compressed samples support
                1.02 - v0.92 added IT compressed delta samples support
*/
#include <stdio.h>
#include "mdriver.h"

MSAMPLE::MSAMPLE(MINPUT *i,ULONG ilength,ULONG ireppos,ULONG irepend,UWORD iflags)
{
    int t;
    data=NULL;
    length=ilength;
    reppos=ireppos;
    repend=irepend;
    flags=iflags;

    if(ilength==0) return;

    MCONVERT converter;

    converter.Init(i,flags,flags|SF_SIGNED);

    if(flags&SF_16BITS){
        data=new SWORD[(length+16)<<1];
    }
    else{
        data=new SBYTE[(length+16)<<1];
    }

    if(data!=NULL){

        /* read sample into buffer. */
        converter.ReadSamples(data,length);

        /* Unclick samples: */

        if(flags & SF_16BITS){
            SWORD *s=(SWORD *)data;

            if(flags & SF_LOOP){
                if(flags & SF_BIDI)
                    for(t=0;t<16;t++) s[repend+t]=s[(repend-t)-1];
                else
                    for(t=0;t<16;t++) s[repend+t]=s[t+reppos];
            }
            else{
                for(t=0;t<16;t++) s[t+length]=0;
            }
        }
        else{
            SBYTE *s=(SBYTE *)data;

            if(flags & SF_LOOP){
                if(flags & SF_BIDI)
                    for(t=0;t<16;t++) s[repend+t]=s[(repend-t)-1];
                else
                    for(t=0;t<16;t++) s[repend+t]=s[t+reppos];
            }
            else{
                for(t=0;t<16;t++) s[t+length]=0;
            }
        }
    }
}


MSAMPLE::~MSAMPLE()
{
    if(flags&SF_16BITS){
        delete[] ((UWORD *)data);
    }
    else{
        delete[] ((UBYTE *)data);
    }
}


void MCONVERT::Init(MINPUT *ii,UWORD iinfmt,UWORD ioutfmt)
{
    in=ii;
    infmt=iinfmt;
    outfmt=ioutfmt;
    oldsample=0;

    /* init compression stuff */
    bytesleft=0;
}



ULONG MCONVERT::getbits(int numbits)
{
    int t;
    ULONG result;

    result=0;

    for(t=0;t<numbits;t++){
    
        if(shiftbits==0){
            shiftbuffer=in->read_UBYTE();
            shiftbits=8;
        }
    
        result>>=1;
        result|=(shiftbuffer&1)<<31;

        shiftbuffer>>=1;
        shiftbits--;
    }

    result>>=(32-numbits);

    return result;
}



void MCONVERT::readwords(SWORD *buffer,int todo)
{
    if(!(infmt & SF_ITCMP)){
        in->read_SWORDS(buffer,todo);
        return;
    }

    while(todo){

        if(bytesleft==0){
            bytesleft=16384;

//          printf("reading at %d ",in->tell());
            compressedblocksize=in->read_UWORD();
//          printf("compressed blocksize %d bytes\n",compressedblocksize);
            bitdepth=17;
            lastword=0;
            loldword=0;
            shiftbuffer=0;
            shiftbits=0;
        }

        int totodo=(todo>bytesleft) ? bytesleft : todo;

        decomp16(buffer,totodo);

        buffer+=totodo;
        todo-=totodo;
        bytesleft-=totodo;
    }
}


void MCONVERT::readbytes(SBYTE *buffer,int todo)
{
    if(!(infmt & SF_ITCMP)){
        in->read_SBYTES(buffer,todo);
        return;
    }

    while(todo){

        if(bytesleft==0){
            bytesleft=32768;

//          printf("reading at %d",in->tell());
            compressedblocksize=in->read_UWORD();
//          printf("compressed blocksize %d bytes\n",compressedblocksize);
            bitdepth=9;
            lastvalue=0;
            loldvalue=0;
            shiftbuffer=0;
            shiftbits=0;
        }

        int totodo=(todo>bytesleft) ? bytesleft : todo;

        decomp8(buffer,totodo);

        buffer+=totodo;
        todo-=totodo;
        bytesleft-=totodo;
    }
}   
    


void MCONVERT::decomp16(SWORD *buffer,int todo)
{
    int t=0;
    ULONG bitvalue;
    SWORD tempvalue;

    while(t<todo){

        bitvalue=getbits(bitdepth);

//      printf("bitdepth %d bitvalue %d ",bitdepth,bitvalue);

        if(bitdepth<7){

            /* bitdepths 1,2,3,4,5,6 */

            if(bitvalue == 1UL<<(bitdepth-1)) {
                UBYTE newbitdepth = (UBYTE)(getbits(4)+1);
                if(newbitdepth >= bitdepth) newbitdepth++;
                bitdepth=newbitdepth;
                continue;
            }
        }
        else if(bitdepth<17){
            
            /* bitdepths 7,8,9,10,11,12,13,14,15,16 */

            UWORD hi=(0xffff>>(17-bitdepth))+8;
            UWORD lo=hi-16;

            if(bitvalue > lo && bitvalue <= hi) {
                UBYTE newbitdepth = (UBYTE)(bitvalue - lo);
                if(newbitdepth >= bitdepth) newbitdepth++;
                bitdepth=newbitdepth;
                continue;
            }
        }
        else if(bitdepth<18){

            /* bitdepth 17 */

            if(bitvalue >= 65536) {
                bitdepth = (UBYTE)(bitvalue - 65535);
                continue;
            }
        }
        else{
//          printf("Error\n");
            t++;
            continue;
        }

        tempvalue=(SWORD)bitvalue;
        if(bitdepth<16){
            tempvalue<<=(16-bitdepth);
            tempvalue>>=(16-bitdepth);
        }

        if(infmt & SF_ITCDL){
            loldword+=tempvalue;
            lastword+=loldword;
        }
        else{
            lastword+=tempvalue;        
        }
        
        buffer[t++]=lastword;
    }
}


    
void MCONVERT::decomp8(SBYTE *buffer,int todo)
{
    int t=0;
    UWORD bitvalue;
    SBYTE tempvalue;

    while(t<todo){

        bitvalue=(UWORD)getbits(bitdepth);

//      printf("bitdepth %d bitvalue %d ",bitdepth,bitvalue);

        if(bitdepth<7){

            /* bitdepths 1,2,3,4,5,6 */
            
            if(bitvalue == 1<<(bitdepth-1)) {
                UBYTE newbitdepth = (UBYTE)(getbits(3)+1);
                if(newbitdepth >= bitdepth) newbitdepth++;
                bitdepth=newbitdepth;
                continue;
            }
        }
        else if(bitdepth<9){

            /* bitdepths 7,8 */

            UWORD hi=(0xff>>(9-bitdepth))+4;
            UWORD lo=hi-8;

            if(bitvalue > lo && bitvalue <= hi) {
                UBYTE newbitdepth = bitvalue - lo;
                if(newbitdepth >= bitdepth) newbitdepth++;
                bitdepth=newbitdepth;
                continue;
            }
        }
        else if(bitdepth<10){

            /* bitdepth 9 */

            if(bitvalue >= 256) {
                bitdepth = bitvalue - 255;
                continue;
            }
        }
        else{
//          printf("Error\n");
            t++;
            continue;
        }

        tempvalue=(SBYTE)bitvalue;
        if(bitdepth<8){
            tempvalue<<=(8-bitdepth);
            tempvalue>>=(8-bitdepth);
        }

        if(infmt & SF_ITCDL){
            loldvalue+=tempvalue;
            lastvalue+=loldvalue;
        }
        else{
            lastvalue+=tempvalue;
        }

        buffer[t++]=lastvalue;
    }
}



void MCONVERT::ReadSamples(void *buf,ULONG length)
{
    int t;
    SBYTE *bptr=(SBYTE *)buf;
    SWORD *wptr=(SWORD *)buf;

//  printf("Loading a %d sample chunk\n",length);
    
    while(length){

        int stodo=(length<1024) ? length : 1024;

        if(infmt & SF_16BITS){
            readwords((SWORD *)buffer,stodo);
        }
        else{
            readbytes((SBYTE *)buffer,stodo);

            SBYTE *s=(SBYTE *)buffer;
            SWORD *d=(SWORD *)buffer;
            s+=stodo;
            d+=stodo;

            for(t=0;t<stodo;t++){
                s--; d--;
                *d=(*s)<<8;
            }
        }

        if(infmt & SF_DELTA){
            for(t=0;t<stodo;t++){
                buffer[t]+=oldsample;
                oldsample=buffer[t];
            }
        }
        
        if((infmt^outfmt) & SF_SIGNED){
            for(t=0;t<stodo;t++){
                buffer[t]^=0x8000;
            }
        }

        if(outfmt & SF_16BITS){
            for(t=0;t<stodo;t++) *(wptr++)=buffer[t];
        }
        else{
            for(t=0;t<stodo;t++) *(bptr++)=buffer[t]>>8;
        }

        length-=stodo;
    }
}



void MCONVERT::ReadBytes(void *buf,ULONG length)
{
    if(outfmt & SF_16BITS) length>>=1;
    ReadSamples(buf,length);
}



MDRIVER::MDRIVER()
{
    tickhandler=NULL;
    uservolume=100;
    ampfactor=1.0;
}



MDRIVER::~MDRIVER()
{
}


void MDRIVER::Tick()
{
    if(tickhandler!=NULL) tickhandler(tickhandlerdata);
}

