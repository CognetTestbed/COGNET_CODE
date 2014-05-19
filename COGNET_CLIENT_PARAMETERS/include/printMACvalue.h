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
#ifndef printMAC
#define printMAC
#ifdef __ANDROID__
#include <android/log.h>
// #define ATH9K_HTC
#endif


#include "observation.h"



#if ATH9K_HTC==1
char *printGlobalValueMac(FILE * fpTot , int ctrlPrintLocal , struct timespec tv, unsigned long long *TOT_PKT, unsigned long long *TOT_BYTE,
     int short_retry, int long_retry, int totretries, int *arrayQueues );
    
#else

char * printGlobalValueMac(FILE * fpTot , int ctrlPrintLocal , struct timespec tv, unsigned long long *TOT_PKT, unsigned long long *TOT_BYTE);

#endif

void printValuesStation(wl_info_t_station *wlinfo);
void printValueStationLog(wl_info_t_station *wlinfo , FILE *fp , struct timespec tv);
char * printValuesStationForApp(wl_info_t_station *wlinfo, struct timespec tv);
void printValueTCPEVENT(tcp_event_info *tcp_event , FILE *fp , struct timespec tv , int ctrlPrintLocal);
#endif
