#include <stdio.h>
//#include <mem.h>
#include <math.h>
#include <assert.h>
#include "mdrv_dsk.h"


MDISK::MDISK() : MDRIVER_FMX()
{
}



MDISK::~MDISK()
{
}



void MDISK::Init()
{
    MDRIVER_FMX::Init();
}



void MDISK::Exit()
{
    MDRIVER_FMX::Exit();
}


void MDISK::Start()
{
    dest=fopen("outfile.raw","wb");
    MDRIVER_FMX::Start();
}


void MDISK::Stop()
{
    MDRIVER_FMX::Stop();
    fclose(dest);
}


void MDISK::Update()
{
    WriteBytes(buffer,2048);
//    fwrite(buffer,2048,1,dest);
}


