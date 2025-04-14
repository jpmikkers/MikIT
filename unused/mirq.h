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
#ifndef MIRQ_H
#define MIRQ_H

#include "mtypes.h"


class MINTERRUPT{

private:
        UWORD vecno;
        UBYTE *thunk;

        static void _saveregs _loadds irqhandler(MINTERRUPT *t);
        void (interrupt far *oldhandler)(void);

protected:
        void chain();

public:
        MINTERRUPT();
        virtual ~MINTERRUPT();

        virtual void handle()=0;

        void Install(UWORD vecno);
        int  IsInstalled();
        void Remove();
};


class MHWINTERRUPT : public MINTERRUPT {

        private:
        UBYTE irq;
        void OnOff(UBYTE irqno,UBYTE onoff);

        protected:
        void EOI();

        public:

        MHWINTERRUPT();
        virtual ~MHWINTERRUPT();

        void Install(UBYTE irq);
        int  IsEnabled(UBYTE irqno);
        int  IsEnabled();
        void Enable();
        void Disable();
};

#endif

