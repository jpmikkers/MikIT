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
#include <unistd.h>
#include "mikit.h"

static int o_noclick= 0;
static int o_interp = 0;
static int o_stereo = 1;
static int o_16bits = 1;
static int o_freq   = 44100; 

long MulDiv(long a,long b,long c)
{
    long double p;
    p=a;
    p*=b;
    p/=c;

    return (long)p;
}

void HandleTick(void *data)
{
    ((MMODULE *)data)->Update();
}


void PlayIT(FILE *fp)
{
    MDRIVER_USS d;
    MINPUT_FP c(fp);
    MMODULE *p;

    d.mode=0;
    if(o_16bits)  d.mode|=DMODE_16BITS;
    if(o_stereo)  d.mode|=DMODE_STEREO;
    if(o_interp)  d.mode|=DMODE_INTERP;
    if(o_noclick) d.mode|=DMODE_NOCLICK;
    d.mode|=DMODE_DITHER;
    d.frequency=o_freq;

    d.channels=32;
    d.latency=200;
    d.subdevice=5;
    d.tickhandler=HandleTick;
    d.tickhandlerdata=NULL;
        
    d.Init();

    if(MMODULE_IT::Test(&c,NULL,0)){
        p=new MMODULE_IT();
        //d.channels=p->GetNumChannels();
        //d.uservolume=200;
    }
    else if(MMODULE_XM::Test(&c,NULL,0)){
        p=new MMODULE_XM();
        //d.channels=p->GetNumChannels();
        //d.uservolume=100;
    }
    else if(MMODULE_MOD::Test(&c,NULL,0)){
        p=new MMODULE_MOD();
        //d.channels=p->GetNumChannels();
        //d.uservolume=100;
    }
    else if(MMODULE_S3M::Test(&c,NULL,0)){
        p=new MMODULE_S3M();
        //d.channels=p->GetNumChannels();
        //d.uservolume=100;
    }
    else{
        throw MikModException("Unknown module type");
    }

    d.tickhandlerdata = p;
    p->Load(&c);

    printf("\nsuccessfully loaded a '%s' type module",p->moduletype);

    d.Start();
    p->SetLoopMode(0);
    p->Start(&d);

    puts("\nNow playing.. hit ctrl-c to stop");

    while(!p->IsReady()){
        d.Update();
    }

    d.Stop();
    p->Stop();
    d.Exit();
    delete p;
}


int main(int argc,char *argv[])
{
    int c;

    puts("MikIT 1.00 Linux - (c)1997 Jean-Paul Mikkers <jpmikkers@gmail.com>\n");
    
    if(argc<2){
        puts("Usage: MIKIT [opts] <module.it>\n\n"
             "With the following options:\n"
             "\n"
             " -n    enable noclick mixing (default is off)\n"
             " -i    enable interpolated mixing (default is off)\n"
             " -8    output 8 bit sound (default: 16 bit)\n"
             " -m    output mono sound (default: stereo)\n"
             " -f x  set mixing rate to 'x' Hz (default: 44100)\n");
        return 1;
    }

    while((c=getopt(argc,argv,"nim8f:"))>=0){
        switch(c){
            case 'n':
                o_noclick=1;
                break;

            case 'i':
                o_interp=1;
                break;

            case 'm':
                o_stereo=0;
                break;

            case '8':
                o_16bits=0;
                break;

            case 'f':
                o_freq=atol(optarg);
                if(o_freq<200 || o_freq>88200){
                    fprintf(stderr,"'%s' is not a valid frequency.\n",optarg);
                    return -1;  
                }
                break;
        }
    }
    
    FILE *fp;

    fp=fopen(argv[optind],"rb");

    if(fp==NULL){
        printf("Can't open '%s'\n",argv[optind]);
        return 1;
    }

    PlayIT(fp);

    fclose(fp);
    return 0;
}

