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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <dirent.h>

#include <unistd.h>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#define STOP_CMD "0:1"

#define DIR_FILE_IP "/sdcard"

int startFunction( int portno , char *filename , char *sleepingTime);
int stopFunction( char * portno , char *filename);
int manageNetworkTestbed(char * filename , char * portno , char * sleepingTime);


int manageNetworkTestbed(char * filename , char * portno , char * sleepingTime){    

    __android_log_print(ANDROID_LOG_DEBUG, "MANAGER", "PORT %s" , portno);    
    startFunction((int)atoi(portno) , filename , sleepingTime);        
    return 0;
}

int stopFunction( char * portno , char *filename){
    startFunction( (int)atoi(portno), filename , STOP_CMD);
    return 0;
}


int startFunction( int portno , char *fileameTmp , char *sleepingTime)
{
	int ii=0;
        int *sockfd, n;
        FILE * fd;                
        char buffer[256];
        char nameIP[25];

        char filename[128];

        int countIP=0;    
        // char *workingDir =getcwd(NULL, 0);                   
        // printf("Open File:%s %s\n", filename , workingDir);

        sprintf(filename, "%s/%s", DIR_FILE_IP , fileameTmp);
       __android_log_print(ANDROID_LOG_DEBUG, "MANAGER", "FILENAME %s" , filename);    
        if ((fd = fopen(filename, "r")) == NULL) {
//                printf("fatal error: cannot open %s\n" , filename);
                error("ERROR opening file");
                __android_log_print(ANDROID_LOG_DEBUG, "MANAGER", "FILENAME ERROR %s" , filename);    
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
                if (sockfd[ii] < 0){
                        __android_log_print(ANDROID_LOG_DEBUG, "MANAGER", "SOCKET CREATION FAILED\n" );   
                        //error("ERROR opening socket");
                        return -1;
                }
                server = gethostbyname(IP[ii]);
                
                if (server == NULL) {
                        fprintf(stderr,"ERROR, no such host\n");
                        return -1;
                }
                
                bzero((char *) &serv_addr, sizeof(serv_addr));
                serv_addr.sin_family = AF_INET;
                bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
                serv_addr.sin_port = htons(portno);
                if (connect(sockfd[ii],(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
                        //error("ERROR connecting");
                    __android_log_print(ANDROID_LOG_DEBUG, "MANAGER", "CONNECTION FAILED\n");   
                        return -1;
                }

                //SEND SLEEPING TIME
                n = write(sockfd[ii],sleepingTime,strlen(sleepingTime));
                
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
