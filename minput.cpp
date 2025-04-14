/*

File:           MINPUT.CPP
Description:    -
Version:        1.00 - original
                1.01 - included a string reading function
                1.02 - added destructor to MINPUT_FN
*/
#include <stdio.h>
#include "minput.h"


MINPUT::MINPUT()
{
        UWORD test=0x0102;
        mode=MOTOROLA;
        native=(((UBYTE *)&test)[0]==0x01) ? MOTOROLA : INTEL;
}


void MINPUT::rewind()
{
        seek(0,SEEK_SET);
}


void MINPUT::wswap(UWORD *b,int n)
{
        while(n--){
                *b=(*b>>8) | (*b<<8);
                b++;
        }
}


void MINPUT::lswap(ULONG *b,int n)
{
        while(n--){
                *b=(*b>>24) |
                   (*b<<24) | 
                   ((*b & 0x00ff0000) >> 8) |
                   ((*b & 0x0000ff00) << 8);
                b++;
        }
}


UWORD MINPUT::read_UWORD()
{
        UWORD result;
        read_UBYTES((UBYTE *)&result,2);
        if(mode!=native) wswap(&result,1);        
        return result;
}


ULONG MINPUT::read_ULONG()
{
        ULONG result;
        read_UBYTES((UBYTE *)&result,4);
        if(mode!=native) lswap(&result,1);        
        return result;
}


int MINPUT::read_UBYTES(UBYTE *b,int n)
{
        while(n--) *(b++)=read_UBYTE();
        return !eof();
}


int MINPUT::read_STRING(char *b,int n)
{
        while(n--){
            char c=read_UBYTE();
            *(b++)=(c) ? c : ' ';
        }
        *(b++)=0;
        return !eof();
}


int MINPUT::read_UWORDS(UWORD *b,int n)
{
        read_UBYTES((UBYTE *)b,n<<1);
        if(mode!=native) wswap(b,n);        
        return !eof();
}


int MINPUT::read_ULONGS(ULONG *b,int n)
{
        read_UBYTES((UBYTE *)b,n<<2);
        if(mode!=native) lswap(b,n);        
        return !eof();
}



/****************************************************************************
class:          MINPUT_MEMORY
derived from:   MINPUT
purpose:        for reading platform independent data from a memory block
****************************************************************************/



MINPUT_MEMORY::MINPUT_MEMORY(void *a,ULONG l) : MINPUT()
{
        if(a==NULL){
                THROW MikModException("NULL memory pointer passed to MINPUT_MEMORY");
        }
        base=(UBYTE *)a;
        end=base+l;
        ptr=base;
}


int MINPUT_MEMORY::eof()
{
        return ( ptr >= end );
}


void MINPUT_MEMORY::seek(long offset,int mode)
{
        switch(mode){
                case SEEK_SET:
                        ptr=base+offset;
                        break;

                case SEEK_CUR:
                        ptr=ptr+offset;
                        break;

                case SEEK_END:
                        ptr=end-offset;
                        break;
        }
        if(ptr>end) ptr=end;
        else if(ptr<base) ptr=base;
}


UBYTE MINPUT_MEMORY::read_UBYTE()
{
        return (ptr<end) ? *(ptr++) : 0;
}



/****************************************************************************
class:          MINPUT_FP
derived from:   MINPUT
purpose:        for reading platform independent data from a filepointer
****************************************************************************/


MINPUT_FP::MINPUT_FP(FILE *f) : MINPUT()
{
        if(f==NULL){
                THROW MikModException("NULL filepointer passed to MINPUT_FP");
        }
        fp=f;
}


int MINPUT_FP::eof()
{
        return feof(fp);
}


void MINPUT_FP::seek(long offset,int mode)
{
        fseek(fp,offset,mode);
}


long MINPUT_FP::tell()
{
    return ftell(fp);
}



UBYTE MINPUT_FP::read_UBYTE()
{
        return fgetc(fp);
}


int MINPUT_FP::read_UBYTES(UBYTE *b,int n)
{
        fread(b,1,n,fp);
        return !eof();
}


/****************************************************************************
class:          MINPUT_FN
derived from:   MINPUT
purpose:        for reading platform independent data from a filepointer
****************************************************************************/


MINPUT_FN::MINPUT_FN(char *s) : MINPUT()
{
        if(s==NULL){
                THROW MikModException("NULL filename passed to MINPUT_FN");
        }
        
        fp=fopen(s,"rb");
        
        if(fp==NULL){
                THROW MikModException("Can't open file");
        }
}


MINPUT_FN::~MINPUT_FN()
{
    if(fp) fclose(fp);
}


int MINPUT_FN::eof()
{
        return feof(fp);
}


void MINPUT_FN::seek(long offset,int mode)
{
        fseek(fp,offset,mode);
}


long MINPUT_FN::tell()
{
    return ftell(fp);
}


UBYTE MINPUT_FN::read_UBYTE()
{
        return fgetc(fp);
}


int MINPUT_FN::read_UBYTES(UBYTE *b,int n)
{
        fread(b,1,n,fp);
        return !eof();
}

