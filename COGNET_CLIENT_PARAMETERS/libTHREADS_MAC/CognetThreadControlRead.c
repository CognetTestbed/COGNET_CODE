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
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>

#include "../include/controlREADDURIP_Thread.h"
#include "../include/commonFunction.h"
#include "../include/macChangeParams.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include <assert.h>



char * itoa(int value){    
    char *s;
    s=(char *)malloc(sizeof(char)*256);
    sprintf(s,"%d" ,value );
    return s;
}

void * handleReadDURIP(void * arg)
{
    int ctrlLoopThreadSocket =1;
    // size_t len=0;
    char buffer[256]; 
    int n;
    int opt = 1;
    int count=0;
    //NEW PART
    int ctrlWiFi;
    int returnValue;
    int ctrlWiFiInfo ;
    int valueChange;
    int ctrlCmd;
    //END NEW PART
    
    char *token, *string, *tofree;
    int words[6];
    
    int sockfd, newsockfd ;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
    #ifdef __ANDROID__
     __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "ERROR opening socket ");
    #endif
    error("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));

    paramThread *param = (paramThread *)arg;
    int portno = atoi(param->port)+1;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR ,&opt , sizeof(int) );
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "ERROR on binding CONTROLREAD CTRL");
#endif
        error("ERROR on binding CONTROLREAD CTRL");

    }
    clilen = sizeof(cli_addr);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "THREAD STARTED %d --- %s " , portno , param->ifname );
#endif
    printf("THREAD STARTED %d --- %s\n" , portno , param->ifname);
    listen(sockfd,5);    
    bzero(buffer ,256);
    while (ctrlLoopThreadSocket == 1) {

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        
        if (newsockfd < 0){
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "ERROR on accept ");
#endif
            error("ERROR on accept");
        }
         
        n = read(newsockfd,buffer,255);
         
        if(n < 1){
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "ERROR to read from socket ");
#endif
            error("ERROR to read from socket");
        }
        printf("RECEIVED CMD:%s\n" , buffer);
        



    tofree = string = strdup(buffer);

    printf("STRING CMD:%s\n" , string);
    assert(string != NULL);
    count = 0;

#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "RECEIVED CMD: %s" , buffer);
#endif

    while ((token = strsep(&string, ":")) != NULL){

        words[count]    = atoi(token);   
        if(count == 0 && words[count] == 2){
      
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "CHANGE FOLDER" );
#else
        printf("CHANGE FOLDER");        
#endif



            token = strsep(&string, ":");
            strcpy(folder , token );
            break;  
        }        
        count++;
    }    
    free(tofree);
        

        
         
        pthread_mutex_lock(&lock);         
        switch(words[0]){
            case 0:
                ctrlLoopGlobal = words[0];         
                ctrlLoopThreadSocket = 0;
                printf("STOP\n");
                n=write(newsockfd,"STOP MACREADER",strlen("STOP MACREADER"));
                pthread_mutex_unlock(&lock);
                break;
             case 1:
                ctrlLoopGlobal = words[0];         
                ctrlCmd = words[1];
                switch(ctrlCmd){
                        case 0:
                            ctrlPrintGlobal  = words[2];
                            n=write(newsockfd,"CHANGE PRINT",strlen("CHANGE PRINT"));
                            pthread_mutex_unlock(&lock);
                            break;
                        
                        case 1:
                            ctrlTSGlobal  = words[2];
                            n=write(newsockfd,"CHANGE TS",strlen("CHANGE TS"));
                            pthread_mutex_unlock(&lock);
                            break;

                        //CASE to get/set wifi parameters like TX POWER TX CHANNEL TX FREQ
                        case 2:
                                pthread_mutex_unlock(&lock);
                                ctrlWiFi = words[2];    
                                printf("GET/SET\n");                                                            
                                switch(ctrlWiFi){        
                                    //GET                            
                                    case 0:
                                        ctrlWiFiInfo = words[3];
                                        
                                        printf("GET\n");  

                                        switch(ctrlWiFiInfo){                                            
                                            case 1:
                                                returnValue = getTXpower(param->ifname);
                                                break;
                                            case 2:
                                                returnValue = getTXfrequency(param->ifname);
                                                break;
                                            case 3:
                                                returnValue = getTXchannel(param->ifname);

                                                break;
                                            default:
                                                returnValue = -1;
#ifdef __ANDROID__
                                                __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "ERROR COMMAND GET WIFI ");
#endif
                                                printf("ERROR COMMAND GET WIFI  \n");
                                        }
                                                
                                        n=write(newsockfd,itoa(returnValue),strlen(itoa(returnValue)));
                                  
                                        break;
                                    //SET
                                    case 1:
                                        printf("SET\n");                                                            
                                        ctrlWiFiInfo = words[3];
                                        valueChange = words[4];                                        
                                        switch(ctrlWiFiInfo){
                                            case 1:
                  #ifdef __ANDROID__                                      
                                                        __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "VALUE TX POWER %d " , valueChange);                                      
                #endif                    
                                                        returnValue = setTXpower(param->ifname, valueChange);

                                                break;
                                            case 2:
                                                        returnValue = setTXfrequency(param->ifname, valueChange);
                                                break;
                                            case 3:
                                                        returnValue = setTXchannel(param->ifname, valueChange);
                                                break;
                                            default:
                                                returnValue = -1;
#ifdef __ANDROID__
                                                __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "ERROR COMMAND SET WIFI   ");
#endif
                                                printf("ERROR COMMAND SET WIFI  \n");
                                        }

                                        if (returnValue == 0)
                                                n=write(newsockfd,"CHANGE OK",strlen("CHANGE OK"));
                                        else
                                                n=write(newsockfd,"ERROR TO CHANGE",strlen("ERROR TO CHANGE"));
                                        break;
                                        // pthread_mutex_unlock(&lock);
                                    default:
#ifdef __ANDROID__
                                        __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "ERROR COMMAND WIFI  ");
#endif
                                        printf("ERROR COMMAND WIFI  \n");
                                        
                                }
                                
                                break;

                                

                        default:
#ifdef __ANDROID__
                        __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "ERROR COMMAND MAC DURIP");
#endif
                        printf("ERROR COMMAND MAC DURIP\n");
                }
                
                break;

                case 2:
                    #ifdef __ANDROID__
                        __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "CREATE FOLDER");
                    #else
                        printf("CREATE FOLDER \n");                
                    #endif
                        
                    pthread_mutex_unlock(&lock);
                    
                break;

             default:
#ifdef __ANDROID__
                __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "COMMAND ERROR THREAD CTRL");
#endif
                printf("COMMAND ERROR THREAD CTRL \n");
         }
         
        // printf("CLOSE ACCEPT\n");
        bzero(words , 6);
        bzero(buffer,256);
        close(newsockfd);

     } /* end of while */
     
    close(sockfd);
#ifdef __ANDROID__
                __android_log_print(ANDROID_LOG_DEBUG, "CONTROLREAD", "CLOSE COGNET MANAGER");
#else
    printf("CLOSE COGNET MANAGER \n");                
#endif

    
    return 0; /* we never get here */
}
