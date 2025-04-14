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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "mikit.h"
#include "mdrv_wav.h"

static void HandleTick(void *data)
{
    ((MMODULE*)data)->Update();
}

static void PlayIT(FILE *fp,FILE *out,int volume)
{
    MMODULE *p;

    try
    {
        MDRV_WAV d(out);
        MINPUT_FP c(fp);

        d.mode=DMODE_16BITS|DMODE_STEREO|DMODE_INTERP|DMODE_NOCLICK|DMODE_DITHER;
        d.channels=64;
        d.frequency=44100;
        d.latency=200;
        d.subdevice=0;
        d.tickhandler=HandleTick;
        d.tickhandlerdata=NULL;
        d.uservolume=volume;

        d.Init();

        d.peak=0;

        if(MMODULE_IT::Test(&c,NULL,0)){
            p=new MMODULE_IT();
        }
        else if(MMODULE_XM::Test(&c,NULL,0)){
            p=new MMODULE_XM();
        }
        else if(MMODULE_MOD::Test(&c,NULL,0)){
            p=new MMODULE_MOD();
        }
        else if(MMODULE_S3M::Test(&c,NULL,0)){
            p=new MMODULE_S3M();
        }
        else if(MMODULE_DCM::Test(&c,NULL,0)){
            p=new MMODULE_DCM();
        }
        else{
            throw MikModException("Unknown module type");
        }

        d.tickhandlerdata = p;

        p->Load(&c);

        printf("\nsuccessfully loaded a %s type module",p->moduletype);

        d.Start();
        p->SetLoopMode(0);
        p->Start(&d);

        puts("\nNow playing.. hit any key to stop");

        while(!p->IsReady()){

            d.Update();

            printf("\rposition: %d",p->GetPos());

            if(_kbhit()){
                _getch();
                break;
            }
        }

        d.Stop();
        p->Stop();
        d.Exit();
        delete p;

    }
    catch(MikModException e)
    {
        printf("MikMod Error: %s\n",e.errmsg);
    }
}



void main(int argc,char *argv[])
{
    puts("MIKIT - (c)1997 Jean-Paul Mikkers <jpmikkers@gmail.com>\n");
    
    if(argc<4){
        puts("Usage: XM2WAV <module.xm> <outfile.wav> <volume>  (standard volume=100)");
        return;
    }

    if(atoi(argv[3])<1 || atoi(argv[3])>2000){
        puts("Illegal volume");
        return;
    }

    FILE *fp;
    FILE *out;

    fp=fopen(argv[1],"rb");

    if(fp==NULL){
        printf("Can't open '%s'\n",argv[1]);
        return;
    }

    out=fopen(argv[2],"wb");

    if(out==NULL){
        printf("Can't open '%s'\n",argv[2]);
        fclose(fp);
        return;
    }

    PlayIT(fp,out,atoi(argv[3]));
    
    fclose(out);
    fclose(fp);
}

