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
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/printMACvalue.h"


#if ATH9K_HTC == 1
char * printGlobalValueMac(FILE * fpTot , int ctrlPrintLocal , struct timespec tv, unsigned long long *TOT_PKT, unsigned long long *TOT_BYTE,
                         int short_retry, int long_retry, int totretries, int *arrayQueues ){
    
    
    char * outputMacAll ;
    outputMacAll = (char *)malloc(sizeof(char ) * 1024);
    sprintf(outputMacAll, "%lu.%lu:RXPACK:%llu:RXB:%llu:TXPACK:%llu:TXB:%llu:SHORTRET:%d:LONGRET:%d:TOTRETRY:%d:"\
"MGMT:%d:CAB:%d:BE:%d:BK:%d:VI:%d:VO:%d:FAILED:%d:QUEUE:%d",
                    (unsigned long) tv.tv_sec,(unsigned long) tv.tv_nsec, TOT_PKT[1], TOT_BYTE[1], TOT_PKT[0],
                    TOT_BYTE[0], short_retry, long_retry, totretries, arrayQueues[0],arrayQueues[1],arrayQueues[2]
                    ,arrayQueues[3],arrayQueues[4],arrayQueues[5],arrayQueues[6],arrayQueues[7] );
    
    switch (ctrlPrintLocal) {
        case 1:        
            printf("%s\n", outputMacAll);            
            break;
        case 2:
            fprintf(fpTot,"%s\n" ,outputMacAll);
            fflush(fpTot);
            break;
        case 3:
            // sprintf(outputMacAll, "%s" ,outputMacAll);            
            fprintf(fpTot, "%s\n" ,outputMacAll);            
            printf("%s\n", outputMacAll);
            fflush(fpTot);
            break;
            
    }
    return outputMacAll;
    
}

#else

char * printGlobalValueMac(FILE * fpTot , int ctrlPrintLocal , struct timespec tv, unsigned long long *TOT_PKT, unsigned long long *TOT_BYTE){
    
    char * outputMacAll ;
    outputMacAll = (char *)malloc(sizeof(char) * 1024);
    

    sprintf(outputMacAll, "%lu.%lu:RXPACK:%llu:RXB:%llu:TXPACK:%llu:TXB:%llu",
                    (unsigned long) tv.tv_sec,(unsigned long) tv.tv_nsec, TOT_PKT[1], TOT_BYTE[1], TOT_PKT[0],
                    TOT_BYTE[0]);
    switch (ctrlPrintLocal) {
        case 1:      
            printf("%s\n", outputMacAll);            
        break;

        case 2:
            fprintf(fpTot, "%s\n" ,outputMacAll);
            fflush(fpTot);
        break;
        case 3:            
            fprintf(fpTot, "%s\n" ,outputMacAll);
            printf("%s", outputMacAll);            
            fflush(fpTot);
        break;
            
    }
    
    return outputMacAll;
    
    
}

#endif

//DA VEDERE L'APP
char * printValuesStationForApp(wl_info_t_station *wlinfo , struct timespec tv){
	
    char *outputStringApp ;
    
//    printf ("__FUNCTION__ = %s\n", __FUNCTION__);

    outputStringApp = (char *)malloc(sizeof(char ) * 1024);
    
	sprintf(outputStringApp, "%lu.%lu:MAC:%s:RSSi:%d:RXB:%llu:RXDROP:%llu:RXDUPL:%llu:RXFRAG:%llu:RXPACKS:%llu:"
                        "TXB:%llu:TXFILT:%llu:TXFRAG:%llu:TXPACKS:%llu:TXRETRYCOUNT:%llu:TXRETRYF:%llu:INMS:%d:BITRATE:%d",
                (unsigned long) tv.tv_sec,(unsigned long) tv.tv_nsec, wlinfo->macAddress , wlinfo->signal_level, wlinfo->rx_bytes, wlinfo->rx_dropped,
                wlinfo->rx_duplicates, wlinfo->rx_fragments, wlinfo->rx_packets, 
                wlinfo->tx_bytes, wlinfo->tx_filtered, wlinfo->tx_fragments, wlinfo->tx_packets,  wlinfo->tx_retry_count,
                wlinfo->tx_retry_failed , wlinfo->inactive_ms  , wlinfo->bitrate          
                );
        
//    printf("%s",outputStringApp);
    return outputStringApp;
}


void printValueTCPEVENT(tcp_event_info *tcp_event , FILE *fp , struct timespec tv , int ctrlPrintLocal)
{

    char * outputTCP_EVENT ;
    outputTCP_EVENT = (char *)malloc(sizeof(char ) * 1024);
    sprintf(outputTCP_EVENT,"%lu.%lu:ACKSEQ:%d:ACKNOSEQ:%d:CWNDR:%d:PKTLOST:%d:LOST_EVENT:%d:BYTES:%d:PKTAC:%d:CLAMP:%d", 
             (unsigned long) tv.tv_sec,(unsigned long) tv.tv_nsec, tcp_event->ACKSEQ,tcp_event->ACKNOSEQ,tcp_event->CWNDR,
              tcp_event->PKTLOST,tcp_event->LOST_EVENT,tcp_event->BYTES,tcp_event->PKTACK,tcp_event->CLAMP);
    


    switch (ctrlPrintLocal) {
        case 1:
            printf("%s\n", outputTCP_EVENT);
            break;
        case 2:
            fprintf(fp,"%s\n", outputTCP_EVENT);
            fflush(fp);
            break;
        case 3:            
            fprintf(fp,"%s\n", outputTCP_EVENT);
            printf("%s\n", outputTCP_EVENT);
            fflush(fp);
            break;
            
    }
   free(outputTCP_EVENT);

}

