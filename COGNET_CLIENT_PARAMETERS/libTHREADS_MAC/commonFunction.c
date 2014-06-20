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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <linux/kd.h>
#include <sys/stat.h>

#include "../include/observation.h"
#include "../include/commonFunction.h"
void error(const char *msg)
{
    perror(msg);
    exit(1);
}




void functionCreateFolder(char * nameExperiment)
{
   struct stat st = {0} ;
   
   char  nameDir[128];
   char  nameDir_TCP[128];
   char  nameDir_MAC[128];
   char  nameDir_MAC_TOT[128];
   char  nameDir_TCP_EVENT[128];
   //nameDir= (char *)malloc(sizeof(char) * (strlen(nameExperiment) + strlen(STRING_PATH_DIR)+1));
   sprintf(nameDir , "%s%s/" ,STRING_PATH_DIR , nameExperiment );
   umask(0);
   if(stat(nameExperiment , &st) == -1 )
   {
       if(mkdir(nameDir , 0666)!=-1){
       #ifdef __ANDROID__
       __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "CREATED FOLDER %s", nameDir);
       #endif
       }else{
       #ifdef __ANDROID__
       __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "ERROR TO CREAT FOLDER %s", nameDir);
       #endif    
       }
       //    nameDir_TCP= (char *)malloc(sizeof(char) * (strlen(nameDir) + strlen(STRING_PATH_TCP)));
       sprintf(nameDir_TCP , "%s%s" ,nameDir , STRING_PATH_TCP );
       
       //nameDir_TCP_EVENT= (char *)malloc(sizeof(char) * (strlen(nameDir) + strlen(STRING_PATH_TCP_EVENT)));
       sprintf(nameDir_TCP_EVENT , "%s%s" ,nameDir , STRING_PATH_TCP_EVENT );
       
       // nameDir_MAC= (char *)malloc(sizeof(char) * (strlen(nameDir) + strlen(STRING_PATH_MAC)));
       sprintf(nameDir_MAC , "%s%s" ,nameDir , STRING_PATH_MAC );
       
       //nameDir_MAC_TOT= (char *)malloc(sizeof(char) * (strlen(nameDir) + strlen(STRING_PATH_MAC_TOT)));
       sprintf(nameDir_MAC_TOT , "%s%s" ,nameDir , STRING_PATH_MAC_TOT );
       
       mkdir(nameDir_TCP , 0666);
       mkdir(nameDir_TCP_EVENT, 0666);
       mkdir(nameDir_MAC , 0666);
       mkdir(nameDir_MAC_TOT, 0666);
       
   }
   else
   {       
       perror("ERROR to create folder");
//       sprintf(nameDir , "%s/%s_COPY" ,STRING_PATH_DIR , nameExperiment );
//       mkdir(nameDir , 0700);
       #ifdef __ANDROID__
       __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "ERROR TO CREAT FOLDER %s", nameDir);
       #endif
   }
   

    
   
}

void findTime(char *s) {
    time_t timenow;
    struct tm *current;
    timenow = time(NULL);
    current = localtime(&timenow);

    int hh = current->tm_hour;
    int mm = current->tm_min;
    int ss = current->tm_sec;

    sprintf(s, "%02d:%02d:%02d", hh, mm, ss);
}



#ifdef __ANDROID__
void checkParameterAppAndroid(char ** argv) {
    __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "PORT:%s", argv[0]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "BUFFER:%s", argv[1]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "PHY: %s", argv[2]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "WLAN: %s", argv[3]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "DIR: %s", argv[4]);
}
#endif
