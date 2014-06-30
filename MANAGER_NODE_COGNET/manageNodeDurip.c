/*
Cognitive Network programm Manager
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#define STOP_CMD "0:1"
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int startFunction( int portno , char *filename , char *sleepingTime);
int manageDURIPFunction( int portno , char *filename , char *cmd);
int manageWifi( int portno , char *filename , char *cmd);
int stopDurip( int portno , char *filename , char *cmd);


int main(int argc, char *argv[]){
    
    int portno;
    char filename[32];
    char buffer[32];
    char *sleepingTime;
    char *cmd;
    int channel, tx;
    if (argc < 2) {
            fprintf(stderr,"usage %s {start,manageMAC,manageWifi,startContinue, stopContinue, changeChannel, changeTX , changeNameFolder}\n", argv[0]);
            exit(0);
    }
    
    
    if(strcmp(argv[1] , "start") == 0 ){
        printf("start MACREADDURIP\n");
        printf("# PORT:\n");
        bzero(buffer,32);
        if(fgets(buffer,31,stdin)==NULL){
                exit(-1);
        }
        portno=atoi(buffer);
        
        printf("FILENAME:\n");
        bzero(buffer,32);
        if(fgets(buffer,31,stdin)==NULL){
                exit(-1);
        }
        strncpy(filename, buffer , strlen(buffer));
        filename[strlen(buffer)-1] = '\0';
        printf("SLEEPING TIME:\n");
        bzero(buffer,32);
        if(fgets(buffer,31,stdin)==NULL){
                exit(-1);
        }
        sleepingTime = buffer;
        startFunction(portno , filename , sleepingTime);
        
    }else{
        if(strcmp(argv[1] , "manageMAC")== 0){
            printf("manageMAC READ DURIP\n");
            printf("# PORT:\n");
            bzero(buffer,32);
            if(fgets(buffer,31,stdin)==NULL){
                exit(-1);
            }
            portno=atoi(buffer);
        
            printf("FILENAME:\n");
            bzero(buffer,32);
            if(fgets(buffer,31,stdin)==NULL){
                exit(-1);
            }
            strncpy(filename, buffer , strlen(buffer));
            filename[strlen(buffer)-1] = '\0';
            printf("CMD MACREADDURIP{0 STOP | 1 CHANGE} { 0 CHANGE PRINT | 2 CHANGE TS}}:{PRINT 0 no feedback | PRINT 1 only video | PRINT 2 only file | PRINT 3 both}|{TS in ms}\n");
            bzero(buffer,32);
            if(fgets(buffer,31,stdin)==NULL){
                exit(-1);
            }
            cmd = buffer;
            manageDURIPFunction(portno , filename , cmd);

        }else{
            if(strcmp(argv[1] , "manageWifi")== 0){
                printf("Manage WIFI\n");
                printf("# PORT:\n");
                bzero(buffer,32);
                if(fgets(buffer,31,stdin)==NULL){
                        exit(-1);
                }
                portno=atoi(buffer);
        
                printf("FILENAME:\n");
                bzero(buffer,32);
                if(fgets(buffer,31,stdin)==NULL){
                        exit(-1);
                }
                strncpy(filename, buffer , strlen(buffer));
                filename[strlen(buffer)-1] = '\0';
                printf("CMD MANAGEWIFI{0 GET | 1 SET }: 1 TXPOWER | 2 TX FREQ | 3 TX CH}:[TXPOWER [0-20] | TXFREQ [2412-2492] | TX CH [1-20]}\n");
                if(fgets(buffer,32,stdin)==NULL){
                    exit(-1);
                }
                cmd  = (char *)malloc(sizeof(char) * strlen(buffer));
                sprintf(cmd,"1:2:%s" , buffer);

                manageWifi(portno , filename , cmd);
            }else{
                if(strcmp(argv[1] , "stop")== 0){
                    printf("STOP READ DURIP\n");
                    printf("# PORT:\n");
                    bzero(buffer,32);
                    if(fgets(buffer,31,stdin)==NULL){
                        exit(-1);
                    }
                    portno=atoi(buffer);
                    printf("FILENAME:\n");
                    bzero(buffer,32);
                    if(fgets(buffer,31,stdin)==NULL){
                        exit(-1);
                    }
                    strncpy(filename, buffer , strlen(buffer));
                    filename[strlen(buffer)-1] = '\0';
                    stopDurip( portno , filename , STOP_CMD);
                }else{
                    if(strcmp(argv[1] , "startContinue")== 0)
                    {
                        portno = atoi(argv[2]);
                        strcpy(filename,argv[3]);
                        sleepingTime = malloc(sizeof(char *) * strlen(argv[4]));
                        strcpy(sleepingTime, argv[4]);
                        startFunction(portno , filename , sleepingTime);
                    }
                    else
                    {
                        if(strcmp(argv[1] , "stopContinue")== 0)
                        {
                            portno = atoi(argv[2]);
                            strcpy(filename,argv[3]);
                            stopDurip( portno , filename , STOP_CMD);                            
                            
                        }else{
                            if(strcmp(argv[1] , "changeChannel")== 0)
                            {
                                portno = atoi(argv[2]);
                                strcpy(filename,argv[3]);                            
                                channel = atoi(argv[4]);
                                cmd  = (char *)malloc(sizeof(char) * strlen(buffer));
                                sprintf(cmd,"1:2:1:3:%d" , channel);
                                manageWifi(portno , filename , cmd);
                           
                            }else{
                                if(strcmp(argv[1] , "changeTX")== 0 ){
                                    portno = atoi(argv[2]);
                                    strcpy(filename,argv[3]);                            
                                    tx = atoi(argv[4]);
                                    cmd  = (char *)malloc(sizeof(char) * strlen(buffer));
                                    sprintf(cmd,"1:2:1:1:%d" , tx);
                                    manageWifi(portno , filename , cmd);


                                }else{    
                                    if(strcmp(argv[1] , "changeNameFolder")== 0 ){
                                    
                                        portno = atoi(argv[2]);
                                        strcpy(filename,argv[3]);                            
                                        cmd  = (char *)malloc(sizeof(char) * (2*strlen(argv[4])));                                        
                                        sprintf(cmd,"2:%s" , argv[4]);                                        
                                        startFunction( portno , filename , cmd);
                                    }else{
                                        fprintf(stderr,"usage %s {start,manageMAC,manageWifi,startContinue, stopContinue, changeChannel, changeTX , changeNameFolder}\n", argv[0]);
                                        exit(0);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }//start
    
    return 0;
}


int stopDurip( int portno , char *filename , char *cmd){
    startFunction( portno , filename , cmd);
    return 0;
}


int manageWifi( int portno , char *filename , char *cmd){
    printf("CMD:%s\n" , cmd);
    startFunction( portno , filename , cmd);
    return 0;    
}

int manageDURIPFunction( int portno , char *filename , char *cmd){
    startFunction( portno , filename , cmd);
    return 0;
    
}





int startFunction( int portno , char *filename , char *sleepingTime)
{
	int ii=0;
        int *sockfd, n;
        FILE * fd;                
        char buffer[256];
        char nameIP[25];

        int countIP=0;    
        char *workingDir =getcwd(NULL, 0);                   
        printf("Open File:%s %s\n", filename , workingDir);
        printf("VALUE:%s" , sleepingTime);
        if ((fd = fopen(filename, "r")) == NULL) {
//                printf("fatal error: cannot open %s\n" , filename);
                error("ERROR opening file");
                return -1;
        }    
        
        
        while(fscanf(fd , "%s", nameIP )!=EOF){
    		if(nameIP[0]!='#'){
    	                countIP++;
    		}
        }        
                      
            rewind(fd);    
            char **IP= (char**)malloc( countIP*sizeof(char *));
            
        while(fscanf(fd , "%s", nameIP )!=EOF){
    		if(nameIP[0]!='#'){
                    	IP[ii]=(char *) malloc(15*sizeof(char));
    	                memcpy(IP[ii] , nameIP , 15);
            	        ii++;
    		}else{
    			printf("NAME PC:%s\n" , nameIP);		
    		}
        }//while
        fclose(fd);    
           
        sockfd = malloc(sizeof(int)*countIP);
    
        for(ii = 0; ii < countIP; ii++){
                struct sockaddr_in serv_addr;
                struct hostent *server;
                /*POTREI TRASFORMARE COME THREAD*/
                printf("IP:%s\n" , IP[ii] );    
                sockfd[ii] = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd[ii] < 0)
                        error("ERROR opening socket");
                
                server = gethostbyname(IP[ii]);
                
                if (server == NULL) {
                        fprintf(stderr,"ERROR, no such host\n");
                        exit(0);
                }
                
                bzero((char *) &serv_addr, sizeof(serv_addr));
                serv_addr.sin_family = AF_INET;
                bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
                serv_addr.sin_port = htons(portno);
                if (connect(sockfd[ii],(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
                        error("ERROR connecting");




                // SEND SLEEPING TIME
                n = write(sockfd[ii], sleepingTime, strlen(sleepingTime));
                
                if (n < 0) 
                        error("ERROR writing to socket");
                bzero(buffer,256);

                n = read(sockfd[ii],buffer,255);
                if (n < 0) 
                        error("ERROR reading from socket");
                printf("IP:%s TIME:%s \n", IP[ii],buffer);
                     
        }

   
    return 0;
}
