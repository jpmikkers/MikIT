/*

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "mikit.h"
#include "mdrv_wav.h"
#include "mdrv_w95.h"

static void HandleTick(void *data)
{
    ((MMODULE*)data)->Update();
}

static void PlayIT(FILE *fp,FILE *out,int volume)
{
    MMODULE *modulePlayer;

    try
    {
        //MDRV_WAV audioDriver(out);
        MDRIVER_W95 audioDriver;
        MINPUT_FP inputStream(fp);

        audioDriver.mode=DMODE_16BITS|DMODE_STEREO|DMODE_INTERP|DMODE_NOCLICK|DMODE_DITHER;
        audioDriver.channels=64;
        audioDriver.frequency=44100;
        audioDriver.latency=200;
        audioDriver.subdevice=0;
        audioDriver.tickhandler=HandleTick;
        audioDriver.tickhandlerdata=NULL;
        audioDriver.uservolume=volume;

        audioDriver.Init();

        audioDriver.peak=0;

        if(MMODULE_IT::Test(&inputStream,NULL,0)){
            modulePlayer=new MMODULE_IT();
        }
        else if(MMODULE_XM::Test(&inputStream,NULL,0)){
            modulePlayer=new MMODULE_XM();
        }
        else if(MMODULE_MOD::Test(&inputStream,NULL,0)){
            modulePlayer=new MMODULE_MOD();
        }
        else if(MMODULE_S3M::Test(&inputStream,NULL,0)){
            modulePlayer=new MMODULE_S3M();
        }
        else if(MMODULE_DCM::Test(&inputStream,NULL,0)){
            modulePlayer=new MMODULE_DCM();
        }
        else{
            throw MikModException("Unknown module type");
        }

        audioDriver.tickhandlerdata = modulePlayer;

        modulePlayer->Load(&inputStream);

        printf("\nsuccessfully loaded a %s type module",modulePlayer->moduletype);

        audioDriver.Start();
        modulePlayer->SetLoopMode(1);
        modulePlayer->Start(&audioDriver);

        puts("\nNow playing.. hit any key to stop");

        while(!modulePlayer->IsReady()){

            audioDriver.Update();

            printf("\rposition: %d",modulePlayer->GetPos());

            if(_kbhit()){
                _getch();
                break;
            }
        }

        audioDriver.Stop();
        modulePlayer->Stop();
        audioDriver.Exit();
        delete modulePlayer;

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
        puts("Usage: MIKIT <module.xm> <outfile.wav> <volume>  (standard volume=100)");
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

