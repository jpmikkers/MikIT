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

File:           MTYPES.H
Description:    Holds various atomic typedefinitions used in MikIT, and system-specific macros
Version:        1.00 - original

*/
#ifndef MTYPES_H
#define MTYPES_H

#ifdef MIKIT_WINDOWS
#include <windows.h>
#endif

#ifdef MIKIT_LINUX
long MulDiv(long a,long b,long c);
#endif

#ifdef MIKIT_FREEBSD
long MulDiv(long a,long b,long c);
#endif

#ifdef MIKIT_BORLAND
// long MulDiv(long a,long b,long c);
#endif

#ifdef MIKIT_WATCOM
long MulDiv(long a,long b,long c);
#pragma aux MulDiv = \
        "imul ebx" \
        "idiv ecx" \
        parm [eax] [ebx] [ecx] \
        value [eax] \
        modify [edx];
#define inportb(x) inp(x)
#define outportb(x,y) outp(x,y)
#define inport(x) inpw(x)
#define outport(x,y) outpw(x,y)
#define disable() _disable()
#define enable() _enable()
#endif


/****************************************************************************
 Atomic mikmod types :
****************************************************************************/

typedef unsigned char UBYTE;
typedef signed char   SBYTE;

typedef unsigned short UWORD;
typedef signed short   SWORD;

typedef unsigned long ULONG;
typedef signed long   SLONG;


/****************************************************************************
 The mikmod error exception :
****************************************************************************/

#ifdef BUGGYEXCEPTIONS

#include <stdio.h>
#include <stdlib.h>

#define BEGINTRY    
#define ENDTRY
#define THROW
#define BEGINCATCH(x)   if(0){
#define ENDCATCH        }

#ifdef MIKIT_EXITHACK
extern void MikIT_ExitHack(char *s);
#define MikModException(x) { MikIT_ExitHack(x); }
#else
#define MikModException(x) { printf("Error: %s\n",x); exit(-1); }
#endif

#else

#define BEGINTRY        try{
#define ENDTRY          }
#define THROW           throw
#define BEGINCATCH(x)   catch(x){
#define ENDCATCH        }

class MikModException{
public:
        char *errmsg;
        MikModException(char *s) : errmsg(s) {}
};

#endif

#endif
