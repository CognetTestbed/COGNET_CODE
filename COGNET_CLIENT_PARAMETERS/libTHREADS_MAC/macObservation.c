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
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <assert.h>

#include <limits.h>

#include "../include/observation.h"
#include "../include/printMACvalue.h"
#include "../include/controlThreadComm.h"
#include "../include/controlREADDURIP_Thread.h"

#include "../include/commonFunction.h"

// #include "../ath9k_htc/htc.h"
// #define ATH9K_HTC 0

#ifdef __ANDROID__
#include <android/log.h>

//WHEN I HAVE AN ANDROID DEVICE WITH DOONGLE
// #define ATH9K_HTC 1
#endif

//ALIX PREPROCESSOR COMPILE
// #define ATH9K_HTC



lookupTableStation * nodes = NULL;



//char *FILE_LIST[]={"last_signal" , "rx_bytes" ,"rx_dropped", "rx_duplicates" ,"rx_fragments","rx_packets",
//		 "tx_bytes" ,"tx_filtered","tx_fragments","tx_packets" , "tx_retry_count" , "tx_retry_failed" , "inactive_ms" };

// Xretries :          8
//    FifoErr :          0
//   Filtered :          0
//   TimerExp :          0
//   SRetries :         87
//   LRetries :        512
//  #stations :          0
//   TOTALRET :        599

char *FILE_LIST_ATH9K_HTC[]={"tgt_tx_stats" , "queue"};
// char *field_ATH9K_HTC[]={"Xretries" , "ShortRetries","LongRetries","TOTALRET","#Nodes" };


//char *field_TCP_EVEN ={"ACKSEQ","ACKNOSEQ","CWNDR","PKTLOST","LOST_EVENT","BYTES""PKTAC","CLAMP"};


/*
PROTOTYPE
*/


int if_getstat(char *ifname, wl_info_t *wlinfo);
int setInfo(void);



// int readDURIP_TCP_EVENT(tcp_event_info *info_tcp)
// {
//     FILE    *fd;    
//     char  tmp[0x100];
//     char *token, *string, *tofree;
//     int count =0;
//     if ((fd = fopen("/proc/tcp_event_durip", "r")) ==NULL) {
//         printf("fatal error: cannot open /proc/tcp_event_durip\n");
//         #ifdef __ANDROID__
//                 __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "ERROR TO OPEN LOG FILE EVENT");
//         #endif
//         return -1;
//     }

// 	info_tcp->ACKSEQ    = 0;   
//     info_tcp->ACKNOSEQ =0;
//     info_tcp->CWNDR     = 0;
//     info_tcp->PKTLOST   = 0;
//     info_tcp->CLAMP     = 0;
//     info_tcp->PKTACK    = 0;
//     info_tcp->BYTES     = 0;
//     info_tcp->LOST_EVENT= 0;
//     // while(fgets(tmp, 0x100, fd))
//     // {
    

//     //     tofree = string = strdup(tmp);
//     //     assert(string != NULL);
//     //     while ((token = strsep(&string, ":")) != NULL){
//     //     	switch(count){
//     //             case 1:
//     //             	info_tcp->ACKSEQ    = atoi(token);   
//     //             break;    
//     //             case 3:
//     //                 info_tcp->ACKNOSEQ  = atoi(token);    
//     //                 break;
//     //             case 5:
//     //                 info_tcp->CWNDR     = atoi(token);
//     //             break;
//     //             case 7:    
//     //                 info_tcp->PKTLOST   = atoi(token);
//     //             break;
//     //             case 9:    
//     //                 info_tcp->LOST_EVENT= atoi(token);
//     //             break;
//     //             case 11:    
//     //                 info_tcp->BYTES     = atoi(token);
//     //             break;
//     //             case 13:    
//     //                 info_tcp->PKTACK    = atoi(token);
//     //             break;
//     //             case 15:    
//     //                 info_tcp->CLAMP     = atoi(token);
//     //             break;	

//     //         }
//     //         count++;
//     //     }
//     // }
//     fclose(fd);
//     // free(tofree);
//     return 0;
// }



/*
STATISTICS FROM /proc/net/dev
*/
int if_getstat(char *ifname, wl_info_t *wlinfo){
    FILE    *fd;
    char    tmp[0x100];
    char    *lp;
    unsigned long d;
    if ((fd = fopen("/proc/net/dev", "r")) ==NULL) {
        printf("fatal error: cannot open /proc/net/dev\n");
                return -1;
    }

    //printf("%s\n" , ifname);
    while (fgets(tmp, 0x100, fd)) {
        	lp = tmp + strspn(tmp, " ");
        	if (!strncmp(lp, ifname, strlen(ifname))) {
			lp += strlen(ifname) + 1;
			lp += strspn(lp, " ");
		//	printf("%s" , tmp);
			sscanf(lp, "%llu %llu %lu %lu %lu %lu %lu %lu %llu  %llu %lu %lu %lu %lu",
			     &wlinfo->Tot_rx_bytes, &wlinfo->Tot_rx_packets, &d, &d, &d, &d, &d, &d,
			     &wlinfo->Tot_tx_bytes, &wlinfo->Tot_tx_packets , &d , &d , &d , &d );
		}

   }

   fclose(fd);
   return 0;
}

#if ATH9K_HTC == 1

int if_getstat_ATH9K_HTC( wl_info_t *wlinfo , char *path){
    FILE    *fd;
    char    tmp[0x100];
    char    *lp;
    char filename[100];
    
    sprintf(filename, "%s%s" , path,FILE_LIST_ATH9K_HTC[0]);
    if ((fd = fopen(filename, "r")) ==NULL) {
        printf("fatal error: cannot open %s\n" , path);
                return -1;
    }
    while (fgets(tmp, 0x100, fd)) {
        	lp = tmp + strspn(tmp, " ");
		// printf("%s\n", lp);
		sscanf(lp, "%u %u %u", &wlinfo->short_retry, &wlinfo->long_retry, &wlinfo->totretries);
   }
   
   fclose(fd);



    sprintf(filename, "%s%s" , path,FILE_LIST_ATH9K_HTC[1]);
    if ((fd = fopen(filename, "r")) ==NULL) {
        printf("fatal error: cannot open %s\n" , path);
                return -1;
    }
    while (fgets(tmp, 0x100, fd)) {
        lp = tmp + strspn(tmp, " ");
        // printf("%s\n", lp);
        sscanf(lp, "%u %u %u %u %u %u %u %u", &wlinfo->arrayQueues[0] , &wlinfo->arrayQueues[1],&wlinfo->arrayQueues[2],&wlinfo->arrayQueues[3],
            &wlinfo->arrayQueues[4],&wlinfo->arrayQueues[5],&wlinfo->arrayQueues[6],&wlinfo->arrayQueues[7]);
   }
   
   fclose(fd);

   return 0;
}


#endif



int counter=0;

void * macObservation(void * param)
{
    
    #ifdef __ANDROID__
    int nn;
    #endif
    char **argv;

    int ctrlLoopLocal;
    int ctrlPrintLocal=0;
    float ctrlTSLocal;
    // struct timespec startingPoint , currentTimePoint;
    
    infoPrintStation infoGetStation;    
    wl_info_t wlinfo ;
    int ctrl_openFile = 0;
    char  ifname[8];
    char path[256];
    char nameFile[128];	
//    FILE *fp = NULL;
	FILE *fpTot= NULL;
	// FILE *fpTCP_EVENT= NULL;
    char *absolutePathFile ;
    
//    int *returnValue=(int *)malloc(sizeof(int));

	int count=0;
	unsigned long long OFFSET_TOT_BYTE[2];
	unsigned long long OFFSET_TOT_PKT[2];

    unsigned long long TOT_BYTE[2];
    unsigned long long TOT_PKT[2];
    char nameExperiment[128];
    // int OFFSET_COLLISION ;

	#if ATH9K_HTC==1
	char path_ATH9K_HTC[150];
	// int OFFSET_xretries ;
	int OFFSET_short_retry ;
	int OFFSET_long_retry ;
	int OFFSET_totretries ;
	#endif
	
	int * returnValue=(int *)malloc(sizeof(int) * 1);
	

	// tcp_event_info  current_TCP_EVENT;
	// tcp_event_info  previous_TCP_EVENT;
 //    tcp_event_info  diff_TCP_EVENT;
    
    
    argv = ((paramThread_MACREAD *)param)->argv;

#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "PORT: %s", argv[0]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "BUFFER: %s", argv[3]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "PHY: %s", argv[4]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "WLAN: %s", argv[5]);
#else
    printf("MACOBSERVATION PORT: %s\n" ,  argv[0]  );
    printf("MACOBSERVATION PRINT: %s\n" ,  argv[1]  );
    printf("MACOBSERVATION TIME: %s\n" ,  argv[2]  );
    printf("MACOBSERVATION PHY: %s\n" ,  argv[3]  );
    printf("MACOBSERVATION WLAN: %s\n" , argv[4]  );
    printf("MACOBSERVATION FOLDER: %s\n" ,argv[5]  );
#endif


    strcpy(ifname ,argv[4]);
    sprintf(path , PATH_MAC , argv[3] , ifname);
    
    infoGetStation.path = (char *)malloc( sizeof(path)+1);
    
    memcpy(infoGetStation.path , path , sizeof(path));
    
    memset(&wlinfo,0,sizeof(wl_info_t));        
   
     #if ATH9K_HTC == 1
        sprintf(path_ATH9K_HTC , PATH_MAC_ATH9K_HTC , argv[3] );
    #endif
//  signal(SIGINT, stopLoop);
	
    char * value;  
	//OPEN LOG FILE!
	time_t timenow;	
	struct tm *current;
	timenow = time(NULL);
	current = localtime(&timenow);
	int month = current->tm_mon+1;
	int day =current->tm_mday;
	int hh_time = current->tm_hour;
	int mm = current->tm_min;
	int ss = current->tm_sec;

    
    strcpy(nameExperiment , argv[5]);
    
    
    ctrlLoopLocal = 1;
    while (ctrlLoopLocal == 1)
    {
     

        // printf("===================================\n");
        // printf("COUNTER:%d\n" , counter);
        // printf("===================================\n");
        if(counter < INT_MAX)
            counter++;
        else
            counter = 0;
// #ifdef __ANDROID__
//         __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "WHILE");
// #endif
        pthread_mutex_lock(&lock);
        ctrlLoopLocal = ctrlLoopGlobal;
        ctrlPrintLocal = ctrlPrintGlobal;
        ctrlTSLocal = ctrlTSGlobal;
        pthread_mutex_unlock(&lock);
                
        /********************************************************/
        /* SECTION FILE                                         */
        /********************************************************/
        
        
        if ((ctrlPrintLocal == 2 || ctrlPrintLocal == 3 ) && ctrl_openFile == 0)
        {
            ctrl_openFile = 1;
            // printf ("__FUNCTION__ = %s\n", __FUNCTION__);
            /*FILE MAC*/
            sprintf(nameFile,NAME_MAC ,day, month, hh_time, mm,ss);
            absolutePathFile = (char *)calloc(( strlen(nameFile) + strlen(STRING_PATH_MAC) + strlen(nameExperiment) + strlen(STRING_PATH_DIR)+2), sizeof(char));
            sprintf( absolutePathFile , "%s%s/%s%s" ,STRING_PATH_DIR, nameExperiment , STRING_PATH_MAC , nameFile );
            if ((infoGetStation.fp = fopen(absolutePathFile, "w")) == NULL)
            {
               
                
#ifdef __ANDROID__
                __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "ERROR TO OPEN LOG FILE:%s", nameFile);
#else
                printf("ERROR TO OPEN LOG FILE MAC %s %d \n" , absolutePathFile , (int)strlen(absolutePathFile));
#endif
                error("MAC");
                *returnValue=-1;
                return returnValue;

            }

            fprintf(infoGetStation.fp, "%d:%d--%d:%d:%d\n", day, month, hh_time, mm, ss);

            
            /*FILE MAC TOT*/
            sprintf(nameFile,NAME_MAC_TOT, day, month, hh_time, mm , ss);
            absolutePathFile = (char *)realloc(absolutePathFile  , sizeof(char) *(strlen(nameFile) + strlen(STRING_PATH_MAC_TOT) + strlen(nameExperiment) + strlen(STRING_PATH_DIR)+2));
            sprintf(absolutePathFile , "%s%s/%s%s" ,STRING_PATH_DIR,nameExperiment , STRING_PATH_MAC_TOT , nameFile );            
            if ((fpTot = fopen(absolutePathFile, "w")) == NULL)
            {
     
               
#ifdef __ANDROID__
                __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "ERROR TO OPEN LOG FILE:%s",
                        nameFile);
#else
               printf("ERROR TO OPEN LOG FILE MACTOT %s %d \n" , absolutePathFile , (int)strlen(absolutePathFile));
#endif          
                error("MACTOT");
                *returnValue=-1;
                return returnValue;
            }//fpTot
            fprintf(fpTot, "%d/%d--%d:%d:%d\n", day, month, hh_time, mm , ss);
            
            /*FILE TCP EVENT*/
//             sprintf(nameFile, NAME_TCP_EVENT, day, month, hh_time, mm, ss);
//             absolutePathFile = (char *)realloc(absolutePathFile, sizeof(char) *(strlen(nameFile) + strlen(STRING_PATH_TCP_EVENT) + strlen(nameExperiment) + strlen(STRING_PATH_DIR)+2));
//             sprintf(absolutePathFile , "%s%s/%s%s" ,STRING_PATH_DIR,nameExperiment , STRING_PATH_TCP_EVENT , nameFile );                                    
//             if ((fpTCP_EVENT = fopen(absolutePathFile, "w")) == NULL)
//             {

// #ifdef __ANDROID__
//                 __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "ERROR TO OPEN LOG FILE:%s", nameFile);
// #else
//                 printf("ERROR TO OPEN LOG FILE TCPEVENT %s %d \n" , absolutePathFile , (int)strlen(absolutePathFile));
// #endif          
                
//                 error("TCP EVENT");
//                 *returnValue=-1;
//                 return returnValue;
//             }
//             fprintf(fpTCP_EVENT, "%d/%d--%d:%d:%d\n", day, month, hh_time, mm, ss);

        }

        /********************************************************/
        /*END SECTION FILE                                         */
        /********************************************************/

        
        
#if ATH9K_HTC == 1
        if_getstat_ATH9K_HTC(&wlinfo, path_ATH9K_HTC);

#else
        if_getstat(ifname, &wlinfo);
#endif


// #ifdef __ANDROID__
//         __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "STATION");
// #endif

        
        clock_gettime(CLOCK_REALTIME, &infoGetStation.gettime_now);             
        infoGetStation.ctrlPrint = ctrlPrintLocal;
        
        get_station(ifname , infoGetStation);
        
        if (count == 0) {

            count=1;
            //0 TX 1 RX
            OFFSET_TOT_BYTE[0] = wlinfo.Tot_tx_bytes;
            OFFSET_TOT_BYTE[1] = wlinfo.Tot_rx_bytes;
            OFFSET_TOT_PKT[0] = wlinfo.Tot_tx_packets;
            OFFSET_TOT_PKT[1] = wlinfo.Tot_rx_packets;
            



#if ATH9K_HTC == 1                                 
            OFFSET_short_retry = wlinfo.short_retry;
            OFFSET_long_retry = wlinfo.long_retry;
            OFFSET_totretries = wlinfo.totretries;                    
            value  = printGlobalValueMac(fpTot, ctrlPrintLocal , infoGetStation.gettime_now , OFFSET_TOT_PKT, OFFSET_TOT_BYTE,
                                OFFSET_short_retry, OFFSET_long_retry, OFFSET_totretries, wlinfo.arrayQueues);

#else
            value  = printGlobalValueMac(fpTot, ctrlPrintLocal , infoGetStation.gettime_now, OFFSET_TOT_PKT, OFFSET_TOT_BYTE);
#endif
    


            // readDURIP_TCP_EVENT(&current_TCP_EVENT);    
            // printValueTCPEVENT(&current_TCP_EVENT , fpTCP_EVENT, infoGetStation.gettime_now, ctrlPrintLocal);            
            
            //  previous_TCP_EVENT = current_TCP_EVENT;

            
#ifdef __ANDROID__
            pthread_mutex_lock(&lock_comm);
            if (socketCommApp != -1){
               int nn = write(socketCommApp,value,strlen(value));    
           }
            pthread_mutex_unlock(&lock_comm);             
#endif
        }else{
            //ELSE after the first read
            unsigned long long current_TOT_TX_BYTE = wlinfo.Tot_tx_bytes - OFFSET_TOT_BYTE[0];
            unsigned long long current_TOT_RX_BYTE = wlinfo.Tot_rx_bytes - OFFSET_TOT_BYTE[1];
            unsigned long long current_TOT_TX_PKT = wlinfo.Tot_tx_packets - OFFSET_TOT_PKT[0];
            unsigned long long current_TOT_RX_PKT = wlinfo.Tot_rx_packets - OFFSET_TOT_PKT[1];
            
            TOT_BYTE[0] = current_TOT_TX_BYTE;
            TOT_BYTE[1] = current_TOT_RX_BYTE;
            TOT_PKT[0] = current_TOT_TX_PKT;
            TOT_PKT[1] = current_TOT_RX_PKT;
            

            
            //CHECK HERE IF IT WILL BE ADD A NEW NODE
            OFFSET_TOT_BYTE[0] = wlinfo.Tot_tx_bytes;
            OFFSET_TOT_BYTE[1] = wlinfo.Tot_rx_bytes;
            OFFSET_TOT_PKT[0] = wlinfo.Tot_tx_packets;
            OFFSET_TOT_PKT[1] = wlinfo.Tot_rx_packets;
            
            
#if ATH9K_HTC == 1
            // int current_TOT_xretries;
            int current_TOT_short_retry;
            int current_TOT_long_retry;
            int current_TOT_totretries;

            if (wlinfo.totretries >= OFFSET_totretries)
            {
                // current_TOT_xretries = wlinfo.Xretries - OFFSET_xretries;
                current_TOT_short_retry = wlinfo.short_retry - OFFSET_short_retry;
                current_TOT_long_retry = wlinfo.long_retry - OFFSET_long_retry;

                current_TOT_totretries = wlinfo.totretries - OFFSET_totretries;
            }
            else
            {

                // current_TOT_xretries = wlinfo.Xretries;
                current_TOT_short_retry = wlinfo.short_retry;
                current_TOT_long_retry = wlinfo.long_retry;
                current_TOT_totretries = wlinfo.totretries;
            }

            
            OFFSET_short_retry = wlinfo.short_retry;
            OFFSET_long_retry = wlinfo.long_retry;
            OFFSET_totretries = wlinfo.totretries;

            
            value  = printGlobalValueMac(fpTot ,ctrlPrintLocal , infoGetStation.gettime_now, TOT_PKT, TOT_BYTE,
                                current_TOT_short_retry, current_TOT_long_retry, current_TOT_totretries, wlinfo.arrayQueues);
            
            
#else
            value  = printGlobalValueMac(fpTot , ctrlPrintLocal , infoGetStation.gettime_now, TOT_PKT, TOT_BYTE);
#endif
            
  
            // //CHECK DI TCP EVENT            
            //  readDURIP_TCP_EVENT(&current_TCP_EVENT);
            //  diff_TCP_EVENT.ACKSEQ = current_TCP_EVENT.ACKSEQ        - previous_TCP_EVENT.ACKSEQ;
            //  diff_TCP_EVENT.ACKNOSEQ = current_TCP_EVENT.ACKNOSEQ    - previous_TCP_EVENT.ACKNOSEQ;
            //  diff_TCP_EVENT.CWNDR = current_TCP_EVENT.CWNDR          - previous_TCP_EVENT.CWNDR;
            //  diff_TCP_EVENT.PKTLOST = current_TCP_EVENT.PKTLOST      - previous_TCP_EVENT.PKTLOST;
            //  diff_TCP_EVENT.LOST_EVENT = current_TCP_EVENT.LOST_EVENT- previous_TCP_EVENT.LOST_EVENT;
            //  diff_TCP_EVENT.BYTES = current_TCP_EVENT.BYTES          - previous_TCP_EVENT.BYTES;
            //  diff_TCP_EVENT.PKTACK = current_TCP_EVENT.PKTACK        - previous_TCP_EVENT.PKTACK;
            //  diff_TCP_EVENT.CLAMP = current_TCP_EVENT.CLAMP;
           

            //  printValueTCPEVENT(&diff_TCP_EVENT , fpTCP_EVENT, infoGetStation.gettime_now, ctrlPrintLocal);
            //  previous_TCP_EVENT = current_TCP_EVENT;
           


//COMMUNICATION SOCKET WITH DURIPAPP
#ifdef __ANDROID__
            pthread_mutex_lock(&lock_comm);
            if (socketCommApp != -1){                        
                int nn=write(socketCommApp,value,strlen(value));    
            }
            pthread_mutex_unlock(&lock_comm);
#endif
            
            
                //CASE TO COMMUNICAT THROUGHT SOCKET TO THE DURIPAPP

                
            if (ctrlPrintLocal >= 2)
            {
                fflush(infoGetStation.fp);
                fflush(fpTot);
            }
        }
        free(value);
        
//        printf("\n=======================================\n");
        usleep(ctrlTSLocal * 1000);
    }//while


    if (ctrlPrintLocal >= 2)
    {

        fclose(infoGetStation.fp);
        fclose(fpTot);
    }
    free(infoGetStation.path);
    free(returnValue);
    printf("CLOSE MAC OBSERVATION\n");    
    return 0;
}//END THREAD
