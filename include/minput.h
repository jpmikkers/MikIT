/*

File:           MINPUT.H
Description:    -
Version:        1.00 - original

*/
#ifndef MINPUT_H
#define MINPUT_H

#include <stdio.h>
#include "mtypes.h"


class MINPUT {

public:
        enum modes { MOTOROLA, INTEL };

protected:
        modes mode;
        modes native;

        void wswap(UWORD *b,int n);
        void lswap(ULONG *b,int n);

public:
        MINPUT();

        void setmode(modes m){
                mode=m;
        }

        modes getmode(){
                return mode;
        }

        void  rewind();

        virtual int   eof()=0;

        virtual void  seek(long offset,int mode)=0;
        virtual long  tell()=0;

        virtual UBYTE read_UBYTE()=0;
        SBYTE read_SBYTE(){ return (SBYTE)read_UBYTE(); }

        UWORD read_UWORD();
        SWORD read_SWORD(){ return (SWORD)read_UWORD(); }

        ULONG read_ULONG();
        SLONG read_SLONG(){ return (SLONG)read_ULONG(); }

        int   read_STRING(char *b,int n);

        virtual int   read_UBYTES(UBYTE *b,int n);
        int   read_SBYTES(SBYTE *b,int n){ return read_UBYTES((UBYTE *)b,n); }

        int   read_UWORDS(UWORD *b,int n);
        int   read_SWORDS(SWORD *b,int n){ return read_UWORDS((UWORD *)b,n); }

        int   read_ULONGS(ULONG *b,int n);
        int   read_SLONGS(SLONG *b,int n){ return read_ULONGS((ULONG *)b,n); }
};



class MINPUT_MEMORY : public MINPUT {
        UBYTE *base,*end,*ptr;
public:
        MINPUT_MEMORY(void *address,ULONG length);
        void seek(long offset,int mode);
        int  eof();
        UBYTE read_UBYTE();
};



class MINPUT_FP : public MINPUT {
        FILE *fp;

public:
        MINPUT_FP(FILE *f);
        void seek(long offset,int mode);
        long tell();
        int  eof();
        UBYTE read_UBYTE();
        int read_UBYTES(UBYTE *b,int n);
};



class MINPUT_FN : public MINPUT {
        FILE *fp;

public:
        MINPUT_FN(char *s);
        ~MINPUT_FN();
        void seek(long offset,int mode);
        int  eof();
        long tell();
        UBYTE read_UBYTE();
        int read_UBYTES(UBYTE *b,int n);
};


#endif