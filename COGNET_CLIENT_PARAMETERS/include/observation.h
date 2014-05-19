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
#ifndef MACPART_H 
#define MACPART_H 

#include "uthash.h"

//ASSOLUTAMENTE DA TOGLIERE
// #define NETMASK "255.255.255.0"
// #define SUBNET "192.168.1.0"

#define IFNAMSIZE        16
#define NUM_PARAMETERS 13
#define ETH_ALEN 6

#define NUM_QUEUE_PARAMS 8

#define PATH_MAC_ATH9K_HTC "/sys/kernel/debug/ieee80211/%s/ath9k_htc/"
#define PATH_MAC "/sys/kernel/debug/ieee80211/%s/netdev:%s/stations/"


// #if MATTEO==1
// #define ATH9K_HTC
// #endif

#ifdef __ANDROID__

    #define STRING_PATH_DIR "/sdcard/local/log/"
    #define ATH9K_HTC 1
#else
    #define STRING_PATH_DIR "/mnt/local/log/"
#endif

#define STRING_PATH_MAC_TOT "log_mac_TOT/"
#define STRING_PATH_MAC "log_mac/"
#define STRING_PATH_TCP "log_tcp/"
#define STRING_PATH_TCP_EVENT "log_tcp_event/"

#define NAME_MAC_TOT "MAC_PARAMS_TOT_%d_%d_%d_%d_%d.log"
#define NAME_MAC "MAC_PARAMS_%d_%d_%d_%d_%d.log"
#define NAME_TCP "TCP_PARAMS_%d_%d_%d_%d_%d.log"
#define NAME_TCP_EVENT "TCP_EVENT_%d_%d_%d_%d_%d.log"

#if ATH9K_HTC==1
typedef struct wl_info{
    /*Wireless info*/
    char name[IFNAMSIZE+1];
    float freq;
    short txpower;
    unsigned long long Tot_rx_packets, Tot_tx_packets;
    unsigned long long Tot_rx_bytes, Tot_tx_bytes;
    // int collision;
    // int Xretries;


    int short_retry ;
    int long_retry ;
    int totretries;
    int arrayQueues[NUM_QUEUE_PARAMS];
    // int bitrate;
}wl_info_t;
#else

typedef struct wl_info{
    /*Wireless info*/
    char name[IFNAMSIZE+1];
    float freq;
    short txpower;
    unsigned long long Tot_rx_packets, Tot_tx_packets;
    unsigned long long Tot_rx_bytes, Tot_tx_bytes;
        
    // int bitrate;
}wl_info_t;

#endif
typedef struct wl_info_station{

    /*Wireless info*/
    char macAddress[18];
    int signal_level;
    unsigned long long rx_packets, tx_packets;
    unsigned long long rx_bytes, tx_bytes;
    unsigned long long rx_duplicates , rx_fragments , rx_dropped;
    unsigned long long tx_filtered, tx_fragments , tx_retry_count, tx_retry_failed;
    int inactive_ms;
    int numParam ;
    int bitrate;

}wl_info_t_station;


typedef struct lookupTableStation{
	char MACADDRESS[18];
	wl_info_t_station prev_Parameters;
	UT_hash_handle hh;
}lookupTableStation;



typedef struct infoPrintStation{
    FILE *fp ;    
    struct timespec gettime_now;    
    int ctrlPrint;
    char *path;
}infoPrintStation;


typedef  struct tcp_event_info{
    int ACKSEQ;
    int ACKNOSEQ;
    int CWNDR;
    int PKTLOST;
    int BYTES;
    int PKTACK;
    int CLAMP;
    int LOST_EVENT;
}tcp_event_info;


typedef struct ipinfo{
  unsigned long int subnet;
  unsigned long int netmask;
}ipinfo;


typedef struct paramThread_tcp {
    char * nameExperiment;
    ipinfo ipNL;
}paramThread_tcp;

int get_station(char *name ,infoPrintStation ctrlPrint);

extern lookupTableStation * nodes;

#endif 