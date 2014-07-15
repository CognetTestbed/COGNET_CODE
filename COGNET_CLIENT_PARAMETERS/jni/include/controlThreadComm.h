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
#ifndef THREAD_COMM_H
#define THREAD_COMM_H

#include <pthread.h>
extern pthread_mutex_t lock_comm;
extern int socketCommApp ;
extern char stationToPlot[18];

extern int sockfdCommDurip;
extern int sock_fd_rcv;

void * handleCommDURIP(void * arg);

typedef struct paramThread_Comm {
    char port[20];
    char ifname[20];    
}paramThread_Comm;

#endif