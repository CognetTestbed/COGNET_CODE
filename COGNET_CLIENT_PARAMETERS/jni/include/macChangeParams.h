/*
Cognitive Network programm 
Copyright (C) 2014  Matteo Danieletto matteo.danieletto@dei.unipd.it
University of Padova, Italy +34 049 827 7778
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef macChangeParam
#define macChangeParam

//#include <sys/types.h>
////#include <sys/socket.h>
//#include <sys/ioctl.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <math.h>
//#include <string.h>
//#include <unistd.h>
////#include <time.h>



//I MUST CHANGE THIS VALUE
//#define IW_NAME "wlan0"

int setTXpower(char *name , int new_txpower);
int setTXfrequency(char *name ,int freq);
int setTXchannel(char *name , int channel);

int getTXpower(char *name);
int getTXfrequency(char *name);
int getTXchannel(char *name);

//int getTXpower();
//int getTXfrequency();
//int getTXchannel();
//int getSensivity();



#define GIGA 1000000000
#define OFFSET_CHANNEL 2412
#define STEP_CHANNEL 5



#endif



