/*
Cognetive Network programm 
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
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>


#include "../include/controlThreadComm.h"
#include "../include/commonFunction.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif




void * handleCommDURIP(void * arg)
{
    int ctrlLoopThreadSocket =1;
//    size_t len=0;
    char buffer[256];
    int n;
    int opt = 1;
    
    int newsockfdLocal ;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    sockfdCommDurip = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfdCommDurip < 0){
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_DEBUG, "CONTROCOMM", "ERROR opening socket ");
#endif
        error("ERROR opening socket");
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    paramThread_Comm *param = (paramThread_Comm *)arg;

    int portno = atoi(param->port)+2;
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    setsockopt(sockfdCommDurip, SOL_SOCKET, SO_REUSEADDR ,&opt , sizeof(int) );
    if (bind(sockfdCommDurip, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_DEBUG, "CONTROCOMM", "ERROR on binding CONTROCOMM CTRL");
#endif
        error("ERROR on binding CONTROCOMM CTRL");
        
    }
    clilen = sizeof(cli_addr);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "CONTROCOMM", "THREAD STARTED %d --- %s " , portno , param->ifname );
#endif
    printf("THREAD STARTED %d --- %s\n" , portno , param->ifname);
    listen(sockfdCommDurip,5);
    while (ctrlLoopThreadSocket) {
        
        newsockfdLocal = accept(sockfdCommDurip, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfdLocal < 0){            
            ctrlLoopThreadSocket=0;
            break;
        }else{
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_DEBUG, "CONTROCOMM", "SOCKET OPENS ");
#endif
            pthread_mutex_lock(&lock_comm);
            socketCommApp=newsockfdLocal;
            //HERE I WILL SUGGEST WHICH ONE STATION I WANT TO PLOT ON GRAPH
            pthread_mutex_unlock(&lock_comm);

            
            n = read(socketCommApp,buffer,255);        
            pthread_mutex_lock(&lock_comm);
    //        sendToSocket=atoi(buffer);
            strcpy(stationToPlot , buffer);
            pthread_mutex_unlock(&lock_comm);
        
            if(n < 1){
    #ifdef __ANDROID__
                __android_log_print(ANDROID_LOG_DEBUG, "CONTROCOMM", "ERROR to read from socket ");
    #endif
                error("ERROR to read from socket");
            }
            
    #ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_DEBUG, "CONTROCOMM", "QUA After: %s------------" , buffer);
    #endif
            // printf("QUA After: %s\n" , buffer);

                    
            //WAIT UNTIL APP SECTION SENDS A SIGNAL TO STOP THE COMM BETWEEN MACOBSERVATION AND APP!
            n = read(socketCommApp,buffer,255);    
          
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_DEBUG, "CONTROCOMM", "RECEIVE CLOSE THREAD COMM: %s\n" , buffer);
#endif
            bzero(buffer,256);
            printf("RECEIVE CLOSE THREAD COMM: %s\n" , buffer);
            
            pthread_mutex_lock(&lock_comm);
            bzero(stationToPlot ,18);
            close(newsockfdLocal);     
            socketCommApp=-1;
            pthread_mutex_unlock(&lock_comm);
        }
        
    } /* end of while */
    
        
    printf("CLOSE COMMUNICATION THREAD\n");
    return 0; /* we never get here */
}
