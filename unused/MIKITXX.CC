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
#include "libmikit.h"

int main(int argc,char *argv[])
{
	int c;

	puts("MikIT 0.91 Linux - (c)1997 Jean-Paul Mikkers <mikmak@via.nl>\n"
		 "See README.DOC for more information\n");

	MikIT_SetChannels(2);
	MikIT_SetResolution(16);
	MikIT_SetMixingFrequency(44100);
	MikIT_SetQuality(0);
	MikIT_SetNoClick(0);
	
	while((c=getopt(argc,argv,"nim8f:"))>=0){
		switch(c){
			case 'n':
				MikIT_SetNoClick(1);
				break;

			case 'i':
				MikIT_SetQuality(1);
				break;

			case 'm':
				MikIT_SetChannels(1);
				break;

			case '8':
				MikIT_SetResolution(8);
				break;

			case 'f':
				{
					int freq=atol(optarg);
					if(freq<2000 || freq>48000){
						fprintf(stderr,"'%s' is not a valid frequency.\n",optarg);
						return -1;	
					}
					MikIT_SetMixingFrequency(freq);
				}
				break;
		}
	}
	
	if(optind==argc){
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

	printf("Loading '%s'\n",argv[optind]);

	if(MikIT_PlayFN(argv[optind])){
		printf("playing.. hit enter to quit. ");
		c=getchar();		
		MikIT_Stop();
	}
	else{
		printf("MikIT Error: %s\n",MikIT_Error());
	}

	return 0;
}

