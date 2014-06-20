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
#include <string.h>
#include <net/if.h>


#include <netlink/utils.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>


#include "../include/observation.h"
#include "../include/nl80211.h"
#include "../include/printMACvalue.h"

#ifdef __ANDROID__

#include "../include/controlThreadComm.h"

#endif



char *FILE_LIST[]={"rx_dropped", "rx_duplicates" ,"rx_fragments","tx_filtered","tx_fragments"};

void parse_bitrate(struct nlattr *bitrate_attr, char *buf, int buflen)
{
	int rate = 0;
	char *pos = buf;
	struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
	static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
		[NL80211_RATE_INFO_BITRATE] = { .type = NLA_U16 },
		[NL80211_RATE_INFO_BITRATE32] = { .type = NLA_U32 },
		[NL80211_RATE_INFO_MCS] = { .type = NLA_U8 },
		[NL80211_RATE_INFO_40_MHZ_WIDTH] = { .type = NLA_FLAG },
		[NL80211_RATE_INFO_SHORT_GI] = { .type = NLA_FLAG },
	};

	if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX,
			     bitrate_attr, rate_policy)) {
		snprintf(buf, buflen, "failed to parse nested rate attributes!");
		return;
	}

	if (rinfo[NL80211_RATE_INFO_BITRATE32])
		rate = nla_get_u32(rinfo[NL80211_RATE_INFO_BITRATE32]);
	else if (rinfo[NL80211_RATE_INFO_BITRATE])
		rate = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
	if (rate > 0)
		pos += snprintf(pos, buflen - (pos - buf),
				"%d.%d MBit/s", rate / 10, rate % 10);

	if (rinfo[NL80211_RATE_INFO_MCS])
		pos += snprintf(pos, buflen - (pos - buf),
				" MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]));
	if (rinfo[NL80211_RATE_INFO_VHT_MCS])
		pos += snprintf(pos, buflen - (pos - buf),
				" VHT-MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_MCS]));
	if (rinfo[NL80211_RATE_INFO_40_MHZ_WIDTH])
		pos += snprintf(pos, buflen - (pos - buf), " 40MHz");
	if (rinfo[NL80211_RATE_INFO_80_MHZ_WIDTH])
		pos += snprintf(pos, buflen - (pos - buf), " 80MHz");
	if (rinfo[NL80211_RATE_INFO_80P80_MHZ_WIDTH])
		pos += snprintf(pos, buflen - (pos - buf), " 80P80MHz");
	if (rinfo[NL80211_RATE_INFO_160_MHZ_WIDTH])
		pos += snprintf(pos, buflen - (pos - buf), " 160MHz");
	if (rinfo[NL80211_RATE_INFO_SHORT_GI])
		pos += snprintf(pos, buflen - (pos - buf), " short GI");
	if (rinfo[NL80211_RATE_INFO_VHT_NSS])
		pos += snprintf(pos, buflen - (pos - buf),
				" VHT-NSS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_NSS]));
}

static char *get_chain_signal(struct nlattr *attr_list)
{
	struct nlattr *attr;
	static char buf[64];
	char *cur = buf;
	int i = 0, rem;
	const char *prefix;

	if (!attr_list)
		return "";

	nla_for_each_nested(attr, attr_list, rem) {
		if (i++ > 0)
			prefix = ", ";
		else
			prefix = "[";

		cur += snprintf(cur, sizeof(buf) - (cur - buf), "%s%d", prefix,
				(int8_t) nla_get_u8(attr));
	}

	if (i)
		snprintf(cur, sizeof(buf) - (cur - buf), "] ");

	return buf;
}


void mac_addr_n2a(char *mac_addr, unsigned char *arg)
{
	int i, l;

	l = 0;
	for (i = 0; i < ETH_ALEN ; i++) {
		if (i == 0) {
			sprintf(mac_addr+l, "%02x", arg[i]);
			l += 2;
		} else {
			sprintf(mac_addr+l, ":%02x", arg[i]);
			l += 3;
		}
	}
}


int FunctionPrint(struct nlattr **sinfo){
    char *chain;
    if (sinfo[NL80211_STA_INFO_INACTIVE_TIME])
		printf("\n\tinactive time:\t%u ms",
			nla_get_u32(sinfo[NL80211_STA_INFO_INACTIVE_TIME]));
	if (sinfo[NL80211_STA_INFO_RX_BYTES])
		printf("\n\trx bytes:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_RX_BYTES]));
	if (sinfo[NL80211_STA_INFO_RX_PACKETS])
		printf("\n\trx packets:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]));
	if (sinfo[NL80211_STA_INFO_TX_BYTES])
		printf("\n\ttx bytes:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_TX_BYTES]));
	if (sinfo[NL80211_STA_INFO_TX_PACKETS])
		printf("\n\ttx packets:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]));
	if (sinfo[NL80211_STA_INFO_TX_RETRIES])
		printf("\n\ttx retries:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_TX_RETRIES]));
	if (sinfo[NL80211_STA_INFO_TX_FAILED])
		printf("\n\ttx failed:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_TX_FAILED]));
    
        
        
    chain = get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL]);
	if (sinfo[NL80211_STA_INFO_SIGNAL])
		printf("\n\tsignal:  \t%d %sdBm",
			(int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]),
			chain);

	chain = get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL_AVG]);
	if (sinfo[NL80211_STA_INFO_SIGNAL_AVG])
		printf("\n\tsignal avg:\t%d %sdBm",
			(int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL_AVG]),
			chain);



	if (sinfo[NL80211_STA_INFO_TX_BITRATE]) {
		char buf[100];

		parse_bitrate(sinfo[NL80211_STA_INFO_TX_BITRATE], buf, sizeof(buf));
		printf("\n\ttx bitrate:\t%s\n", buf);
	}
    return 0;
}


void prepareStationToPrint(wl_info_t_station *tmpStationParam , wl_info_t_station prevStatParam )
{
    int zz;
    for (zz = 1; zz < NUM_PARAMETERS - 1; zz++)
    {
        switch (zz)
        {
            case 1:
                tmpStationParam->rx_bytes           = tmpStationParam->rx_bytes         - prevStatParam.rx_bytes;
                break;
            case 2:
                tmpStationParam->rx_dropped         = tmpStationParam->rx_dropped       - prevStatParam.rx_dropped;
                break;
            case 3:
                tmpStationParam->rx_duplicates      = tmpStationParam->rx_duplicates    - prevStatParam.rx_duplicates;
                break;
            case 4:
                tmpStationParam->rx_fragments       = tmpStationParam->rx_fragments     - prevStatParam.rx_fragments;
                break;
            case 5:
                tmpStationParam->rx_packets         = tmpStationParam->rx_packets       - prevStatParam.rx_packets;
                break;
            case 6:
                tmpStationParam->tx_bytes           = tmpStationParam->tx_bytes         - prevStatParam.tx_bytes;
                break;
            case 7:
                tmpStationParam->tx_filtered        = tmpStationParam->tx_filtered      - prevStatParam.tx_filtered;
                break;
            case 8:
                tmpStationParam->tx_fragments       = tmpStationParam->tx_fragments     - prevStatParam.tx_fragments;
                break;
            case 9:
                tmpStationParam->tx_packets         = tmpStationParam->tx_packets       - prevStatParam.tx_packets;
                break;
            case 10:
#if ATH9K_HTC == 1
                tmpStationParam->tx_retry_count     = tmpStationParam->tx_retry_count   - prevStatParam.tx_retry_count;
#else
				tmpStationParam->tx_retry_count     = 0;
#endif
                break;
            case 11:
#if ATH9K_HTC == 1
                tmpStationParam->tx_retry_failed    = tmpStationParam->tx_retry_failed  - prevStatParam.tx_retry_failed;
#else
				tmpStationParam->tx_retry_failed    = 0;   
#endif        
                break;                
        }
    }
}





int if_getstatStation(struct nlattr **sinfo ,wl_info_t_station *wlinfo , char *path)
{
        
	// printf("__FUNCTION__ %s \n", __FUNCTION__ );
    int ii;
    FILE    *fd;
	char  tmp[100];
	char  fileOpen[100];
    
    if (sinfo[NL80211_STA_INFO_INACTIVE_TIME]){        
         wlinfo->inactive_ms = nla_get_u32(sinfo[NL80211_STA_INFO_INACTIVE_TIME]);
    }
	if (sinfo[NL80211_STA_INFO_RX_BYTES]){
         wlinfo->rx_bytes = nla_get_u32(sinfo[NL80211_STA_INFO_RX_BYTES]);
    }
	if (sinfo[NL80211_STA_INFO_RX_PACKETS]){
        wlinfo->rx_packets = 	nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]);        
    }
	if (sinfo[NL80211_STA_INFO_TX_BYTES]){
        wlinfo->tx_bytes = nla_get_u32(sinfo[NL80211_STA_INFO_TX_BYTES]);            
    }
	if (sinfo[NL80211_STA_INFO_TX_PACKETS]){            
        wlinfo->tx_packets = nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]);
    }
#if ATH9K_HTC == 0
	if (sinfo[NL80211_STA_INFO_TX_RETRIES]){           
        wlinfo->tx_retry_count = nla_get_u32(sinfo[NL80211_STA_INFO_TX_RETRIES]);
    }
	if (sinfo[NL80211_STA_INFO_TX_FAILED]){
        wlinfo->tx_retry_failed = 	nla_get_u32(sinfo[NL80211_STA_INFO_TX_FAILED]);
    }
#endif

    get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL]);
	if (sinfo[NL80211_STA_INFO_SIGNAL]){
        wlinfo->signal_level = (int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]);
    }
    
    
	if (sinfo[NL80211_STA_INFO_TX_BITRATE]) {
		char buf[100];

		parse_bitrate(sinfo[NL80211_STA_INFO_TX_BITRATE], buf, sizeof(buf));
		wlinfo->bitrate=atoi(buf);
		// printf("\n\ttx bitrate:\t%s\n", buf);
	}


// {"rx_dropped", "rx_duplicates" ,"rx_fragments","tx_filtered","tx_fragments"}
    for(ii=0; ii< 5; ii++){
        
        strcpy(fileOpen , path);	
        strcat(fileOpen, FILE_LIST[ii]);	
        if ((fd = fopen(fileOpen, "r")) == NULL)
        {
            printf("fatal error: %s\n" , fileOpen);
            return -1;
        }

        if (fgets(tmp, 100, fd) == NULL)
        {
        	printf("fatal error: %s\n" , FILE_LIST[ii]);
        }
        fclose(fd);
        switch(ii)
        {   		
            case 0:
                sscanf(tmp, "%llu" , &wlinfo->rx_dropped);		
            break;
            case 1:	
                sscanf(tmp, "%llu" , &wlinfo->rx_duplicates);		
            break;
            case 2:
                sscanf(tmp, "%llu" , &wlinfo->rx_fragments);		
            break;   		
            case 3:
                sscanf(tmp, "%llu" , &wlinfo->tx_filtered);		
            break;
            case 4:
                sscanf(tmp, "%llu" , &wlinfo->tx_fragments);		
            break;
   		
        }
    }
    
  							
	return 0;
		
}






int UpdateStation(struct nlattr **sinfo , char *tmpName ,infoPrintStation *info ){
    
    // printf("__FUNCTION__ %s \n", __FUNCTION__ );

    lookupTableStation  *nodes_iter;
    wl_info_t_station tmpStationParam , tmpStationParamOLD ;
    strcpy(tmpStationParam.macAddress,tmpName);

	int length = strlen(info->path)+strlen(tmpName)+2;
    char * path = (char *)calloc(length, sizeof(char));
    sprintf(path,"%s%s/" ,info->path , tmpName ); 

    if_getstatStation(sinfo, &tmpStationParam , path);
    
    HASH_FIND_STR(nodes, tmpName, nodes_iter);
    if (nodes_iter)
    {
        memcpy(&tmpStationParamOLD ,&tmpStationParam , sizeof(wl_info_t_station) );
        wl_info_t_station prevStatParam = nodes_iter->prev_Parameters;
        prepareStationToPrint( &tmpStationParam , prevStatParam );        
        // nodes_iter->prev_Parameters = tmpStationParam;
        nodes_iter->prev_Parameters = tmpStationParamOLD;

    }else{
        nodes_iter = (lookupTableStation*) malloc(sizeof (lookupTableStation));
        strcpy(nodes_iter->MACADDRESS, tmpName);
        nodes_iter->prev_Parameters = tmpStationParam;        
        HASH_ADD_KEYPTR(hh, nodes, nodes_iter->MACADDRESS, strlen(nodes_iter->MACADDRESS), nodes_iter);       
        
        
		// free(nodes_iter);
    }
    
    
          
    char * stringToPrint = printValuesStationForApp(&tmpStationParam , info->gettime_now);
       
    switch(info->ctrlPrint){
        case 1:
#ifdef __ANDROID__
            // __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "%s",stringToPrint);
#else
            printf("%s\n" , stringToPrint);
#endif
        break;
        
        case 2:
            fprintf(info->fp,"%s\n" ,stringToPrint );
        break;
            
        case 3:
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "%s",stringToPrint);
#else
            printf("%s" , stringToPrint);
#endif
            
            fprintf(info->fp,"%s\n" ,stringToPrint );
        break;
    }
    
    
#ifdef __ANDROID__                
    pthread_mutex_lock(&lock_comm);
    if ((strncmp(tmpName , stationToPlot , strlen(tmpName))==0))
    {
		char * stringToPass = (char *)malloc(sizeof(char) *(strlen(stringToPrint)+2));
		sprintf(stringToPass,"%s\n", stringToPrint);			
        if(write(socketCommApp,stringToPass,strlen(stringToPass))==-1){
        	__android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "MAC SEL SOCKET ERROR\n");			
        }
        
        __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "MAC SEL %s",stringToPass);
        free(stringToPass);
    }
    else
    {
        // __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "NO %s--%s -- %d  -- %d\n",stationToPlot , tmpName, strlen(stationToPlot) , strlen(tmpName));
    }

    pthread_mutex_unlock(&lock_comm);

#endif   

	
   	free(path);
    free(stringToPrint);
    return 0;
}




static int station(struct nl_msg *msg, void *arg){
    
    // printf("__FUNCTION__ %s \n", __FUNCTION__ );
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
	char mac_addr[20],  dev[20];
//    printf ("__FUNCTION__ = %s\n", __FUNCTION__);
    infoPrintStation * ctrlPrint = (infoPrintStation *)arg;
    
//	struct nl80211_sta_flag_update *sta_flags;
//        printf("QUANTE VOLTE:%d\n" , NL80211_STA_INFO_MAX);
	static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
		[NL80211_STA_INFO_INACTIVE_TIME] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_BYTES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_BYTES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
		[NL80211_STA_INFO_T_OFFSET] = { .type = NLA_U64 },
		[NL80211_STA_INFO_TX_BITRATE] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_RX_BITRATE] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_LLID] = { .type = NLA_U16 },
		[NL80211_STA_INFO_PLID] = { .type = NLA_U16 },
		[NL80211_STA_INFO_PLINK_STATE] = { .type = NLA_U8 },
		[NL80211_STA_INFO_TX_RETRIES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_FAILED] = { .type = NLA_U32 },
		[NL80211_STA_INFO_STA_FLAGS] =
			{ .minlen = sizeof(struct nl80211_sta_flag_update) },
		[NL80211_STA_INFO_LOCAL_PM] = { .type = NLA_U32},
		[NL80211_STA_INFO_PEER_PM] = { .type = NLA_U32},
		[NL80211_STA_INFO_NONPEER_PM] = { .type = NLA_U32},
		[NL80211_STA_INFO_CHAIN_SIGNAL] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_CHAIN_SIGNAL_AVG] = { .type = NLA_NESTED },
	};
	

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	/*
	 * TODO: validate the interface and mac address!
	 * Otherwise, there's a race condition as soon as
	 * the kernel starts sending station notifications.
	 */

	if (!tb[NL80211_ATTR_STA_INFO]) {
		fprintf(stderr, "sta stats missing!\n");
		return NL_SKIP;
	}
	if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX,
			     tb[NL80211_ATTR_STA_INFO],
			     stats_policy)) {
		fprintf(stderr, "failed to parse nested attributes!\n");
		return NL_SKIP;
	}

	mac_addr_n2a(mac_addr, nla_data(tb[NL80211_ATTR_MAC]));
	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);

//	printf("Station %s (on %s)\n", mac_addr, dev);
//QUI FACCIO IL CHECK se la station e' presente    
//    testFunctionPrint(sinfo);
	// printf ("__FUNCTION__ = %s\n", __FUNCTION__);
    UpdateStation(sinfo ,mac_addr, ctrlPrint);
	        
    return 0;
}


int get_station(char *name ,infoPrintStation ctrlPrint){
// printf ("__FUNCTION__ = %s\n", __FUNCTION__);
	long long int devidx = if_nametoindex(name);    
//        printf("%lld\n" , devidx);	
    struct nl_sock *sock;
 	
	struct nl_msg *msg;
	int family;


	int rate = 0;

	sock = nl_socket_alloc();
	
	msg = nlmsg_alloc();
	// printf("ERROR LINE 498\n");	
	// printf("__FUNCTION__ %s \n", __FUNCTION__ );
	if(genl_connect(sock) <0)
		printf("ERROR genl_connect\n");
	// else
	// 	printf("ERROR LINE 500\n");
	
	family = genl_ctrl_resolve(sock, "nl80211");
	// printf("ERROR LINE 502\n");
	
	genlmsg_put(msg, 0, 0, family, 0, NLM_F_DUMP, NL80211_CMD_GET_STATION, 0);        
	nla_put_u32(msg, NL80211_ATTR_IFINDEX, devidx);


	if(nl_send_auto_complete(sock, msg) < 0)
		goto cleanup;

 	nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, station, &ctrlPrint); 
    
    
	nl_recvmsgs_default(sock);
//printf("==============================================================\n");
cleanup:
//    printf("CLEANUP\n");
    nl_socket_free(sock);
    nlmsg_free(msg);

	return rate;
 }


