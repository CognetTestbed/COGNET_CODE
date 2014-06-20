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

//Questo header e' il thread di controllo del processo di readDurip
// si occupa di leggere e/o cambiare i parametri di macReadDurip (macObservation.c) e tcpReadDurip (tcpObservation.c)
#ifndef THREAD_H 
#define THREAD_H

extern int ctrlLoopGlobal;
extern int ctrlPrintGlobal;
extern float ctrlTSGlobal;
extern pthread_mutex_t lock;


void * handleReadDURIP(void * arg);


typedef struct paramThread {
    char port[20];
    char ifname[20];
}paramThread;

typedef struct paramThread_MACREAD {
    int argc;
    char ** argv;

}paramThread_MACREAD;

typedef struct paramThread_TCPREAD {
    int argc;
    void * argv;

}paramThread_TCPREAD;
//QUI DA AGGIUNGERE TUTTE LE VARIABILI GLOBALI DI CONTROLLO DEI THREAD macObservation e tcpObservation

void * macObservation(void * param);
void * tcpObservation(void * param);
#endif