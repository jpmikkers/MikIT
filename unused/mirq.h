/*

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

