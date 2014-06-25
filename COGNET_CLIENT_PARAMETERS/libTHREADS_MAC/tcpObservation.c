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
# include <sys/types.h>
# include <sys/socket.h>
# include <linux/netlink.h>
# include <stdlib.h>
# include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include "../include/observation.h"
#include "../include/commonFunction.h"
#include "../include/controlREADDURIP_Thread.h"

#define NETLINK_USER_SND 21
#define NETLINK_USER_RCV 31

#define MAX_PAYLOAD 1024 /* maximum payload size*/

#define BUFFER_DIM  1024*1024
int sock_fd_snd , sock_fd_rcv ;


struct tcpEventCollection{    
    __u8 countLOSS_EVENT ;
    __u8 countCWND_RESTART;
    __u8 countACKSEQ;
    __u8 countACKNOSEQ;    
    __u8 countFASTRECOVERYENDED;
};


typedef struct tcp_log {
    // struct timespec tv;
    __s64 delta;
    __be32 saddr, daddr;
    __be16 sport, dport;           
    __u32  snd_cwnd;
    __u32  rcv_wnd;        /* Current receiver window              */
    __u32  ssthresh;
    __u32  rtt;
    __u32  srtt;    
    __u32  srtt_jtu;
    __u32  rto;   
    __u32  rttvar;
    __u32  in_flight;        /*in_flight = packets_out - left_out + retrans_out*/
    __u32  packets_out;    /*  packets_out is SND.NXT-SND.UNA counted in packets.        */
    __u32  retrans_out;    /* Retransmitted packets out     retrans_out is number of retransmitted segments*/
    __u32  lost_out;       /* Lost packets                 */
    __u32  total_retrans;
    __u32  bytes_acked;  
    __u32  numpktacked;    
    __u32  cachemss;
    __u32  sacked_out;
    __u32  snd_una;        // First byte we want an ack for        oldest unacknowledged sequence number
    __u32  snd_nxt;        // Next sequence we send                next sequence number to be sent
    __u32  ack_seq;        /* Sequence number ACK'd        */
    __u8   frto_counter;   /* Number of new acks after RTO */    
    __u8   backoff;    
    __u32  last_data_sent ;
    __u32  last_data_recv;
    __u32  last_ack_recv;
    __u8   reordering; /* Packet reordering metric.*/
    __u8   tcpCAState;
    __u32  prr_delivered;  /* Number of newly delivered packets to  receiver in Recovery. */
    __u32  prr_out;        /* total packet sent during recovery*/

    struct tcpEventCollection tcpEvent;
}tcp_log;







struct msghdr msg_snd;
struct msghdr msg_rcv;


//struct nlmsghdr * createNLM(){
void createNLM(struct nlmsghdr * nlh){
//    struct nlmsghdr *nlh=NULL ;
//    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
    
//	return nlh;
}



//struct sockaddr_nl createDestAddr(){
//    struct sockaddr_nl dest_addr;
//    memset(&dest_addr, 0, sizeof(dest_addr));
////    memset(&dest_addr, 0, sizeof(dest_addr));
//    dest_addr.nl_family = AF_NETLINK;
//    dest_addr.nl_pid = 0; /* For Linux Kernel */
//    dest_addr.nl_groups = 0; /* unicast */
//    return dest_addr;
//}


void createDestAddr(struct sockaddr_nl * dest_addr){
       

    dest_addr->nl_family = AF_NETLINK;
    dest_addr->nl_pid = 0; /* For Linux Kernel */
    dest_addr->nl_groups = 0; /* unicast */
    
}







int keepRunningTCP =1;
void closeThread(int ii) {
    keepRunningTCP = 0;
    // close(sockfdMain);   
    printf("CLOSE THREAD TCP \n");
    close(sock_fd_rcv);     
}



void * tcpObservation(void * param){

    FILE *fp;
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh_snd=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD)) ;
    struct nlmsghdr *nlh_rcv=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD)) ;
    struct iovec iov_snd;
    struct iovec iov_rcv;

    // socklen_t optlen;
    int recvbuff;

    paramThread_tcp * paramThreadTCP;
    
    int ctrlLoopLocal;
    int ctrlPrintLocal;
    char ipbuf[INET_ADDRSTRLEN];	   
           
    int firstACK;
    char nameFile[128];
    int *returnValue=0;
    
	//OPEN LOG FILE!
	time_t timenow;

    struct timeval  tv;
    
    struct timezone tz;

    struct tm      *tm;
	struct tm *current;
	timenow = time(NULL);
	current = localtime(&timenow);
	int month = current->tm_mon+1;
    int day =current->tm_mday;
	int hh = current->tm_hour;
	int mm = current->tm_min;
	int ss = current->tm_sec;
    
    

    
    // struct sockaddr_in ip4addr;
    // struct sockaddr_in ip4addrMASK;
    char nameExperiment[32];
    char *absolutePathFile;
    char printString[1024];


    // timenow = time(NULL);
    // current = localtime(&timenow);
    // month = current->tm_mon+1;
    // day =current->tm_mday;
    // hh = current->tm_hour;
    // mm = current->tm_min;
    // ss = current->tm_sec;


    

    paramThreadTCP = (paramThread_tcp *)param;
    strcpy(nameExperiment , paramThreadTCP->nameExperiment);
    
    memset(&dest_addr, 0, sizeof(dest_addr));
    // printf("%s\n" , nameExperiment);    
    sprintf(nameFile, NAME_TCP, day, month, hh, mm,ss);
    int lenghtName =(strlen(nameFile) + strlen(STRING_PATH_TCP) + strlen(nameExperiment) + strlen(STRING_PATH_DIR) + 2);
    absolutePathFile = (char *)malloc(sizeof(char) * lenghtName);
    sprintf(absolutePathFile , "%s%s/%s%s" ,STRING_PATH_DIR, nameExperiment , STRING_PATH_TCP , nameFile );                                    
    

    // signal(SIGINT,closeThread);
// SIGTERM
    signal(SIGTERM,closeThread);
	printf("THREAD STARTED %s\n" , __FUNCTION__);
    if((fp = fopen(absolutePathFile, "w")) == NULL){
		printf("ERROR TO OPEN LOG FILE %s\n" , absolutePathFile);
		*returnValue=-1;
    	error("Open FILE TCP");
		return returnValue;
	}
    
	fprintf(fp,"%d/%d--%d:%d:%d\n" , day , month , hh , mm, ss);
	    
   	

	// ipNL.subnet = ip4addr.sin_addr.s_addr;
	// ipNL.netmask = ip4addrMASK.sin_addr.s_addr;
	
	sock_fd_snd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER_SND);
	if(sock_fd_snd<0){
        	*returnValue=-1;
        	return returnValue;
    	}
    
	sock_fd_rcv = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER_RCV);
	if(sock_fd_rcv<0){
        	*returnValue=-1;
        	return returnValue;
    	}
	
    
    //getsockopt(sock_fd_rcv , SOL_SOCKET , SO_RCVBUF , &recvbuff , &optlen);

    //  printf("rcv buffer size = %d\n", recvbuff);
    recvbuff = BUFFER_DIM;
    setsockopt(sock_fd_rcv, SOL_SOCKET, SO_RCVBUFFORCE, &recvbuff, sizeof(recvbuff));


	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid(); /* self pid */
    
	bind(sock_fd_snd, (struct sockaddr*)&src_addr, sizeof(src_addr));
	bind(sock_fd_rcv, (struct sockaddr*)&src_addr, sizeof(src_addr));
    

 	createDestAddr(&dest_addr);
 	createNLM(nlh_snd);

    
	//strcpy(NLMSG_DATA(nlh_snd), "OPEN CONNECTION SEND");
    
	memcpy(NLMSG_DATA(nlh_snd), &(paramThreadTCP->ipNL), sizeof(paramThreadTCP->ipNL));
	iov_snd.iov_base = (void *)nlh_snd;
	iov_snd.iov_len = nlh_snd->nlmsg_len;
    
    
    	//MOLTO IMPORTANTE SND
	msg_snd.msg_name = (void *)&dest_addr;
	msg_snd.msg_namelen = sizeof(dest_addr);
	msg_snd.msg_iov = &iov_snd;
	msg_snd.msg_iovlen = 1;
    
    
	sendmsg(sock_fd_snd,&msg_snd,0);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "TCPOBSERVATION", "Sending message to kernel TO OPEN SEND CONNECTION DATA TO KERNEL ");
#else
	printf("Sending message to kernel TO OPEN SEND CONNECTION DATA FROM APP TO KERNEL \n");
#endif
    
    
	createNLM(nlh_rcv);
	strcpy(NLMSG_DATA(nlh_rcv), "OPEN CONNECTION RECV");
    
	iov_rcv.iov_base = (void *)nlh_rcv;
	iov_rcv.iov_len = nlh_snd->nlmsg_len;
    
    //MOLTO IMPORTANTE RCV
	msg_rcv.msg_name = (void *)&dest_addr;
	msg_rcv.msg_namelen = sizeof(dest_addr);
	msg_rcv.msg_iov = &iov_rcv;
	msg_rcv.msg_iovlen = 1;
    
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "TCPOBSERVATION", "Sending message to kernel TO OPEN RECEIVE CONNECTION DATA FROM KERNEL");
#else
	printf("Sending message to kernel TO OPEN RECEIVE CONNECTION DATA FROM KERNEL \n");
#endif
       
	sendmsg(sock_fd_rcv,&msg_rcv,0);
    


    pthread_mutex_lock(&lock);
    ctrlLoopLocal = ctrlLoopGlobal;
    ctrlPrintLocal = ctrlPrintGlobal;
    pthread_mutex_unlock(&lock);
    

     
    tcp_log *tp;  

    firstACK = 1;


    /*
    *WHILE ACK
    */

    while(ctrlLoopLocal){

	tp =  (tcp_log *)NLMSG_DATA(nlh_rcv);
	memset(nlh_rcv, 0, NLMSG_SPACE(MAX_PAYLOAD));                
        		
        if (recvmsg(sock_fd_rcv, &msg_rcv, 0 ) > 0){

    	    inet_ntop(AF_INET, &tp->daddr, ipbuf, INET_ADDRSTRLEN);
            unsigned int subnetComputed = (tp->daddr) & paramThreadTCP->ipNL.netmask;
            if( subnetComputed == paramThreadTCP->ipNL.subnet ){

                

                
                    
            sprintf(printString , "%lld:IP:%s:CWND:%010d:AW:%010d:SSHTHR:%u:\
RTT:%u:RTTV:%u:SRTT:%u:SRTTJTU:%u:RTO:%u:\
FLIGHT:%u:PO:%u:PL:%u:RO:%u:PRRDEV:%u:PRROUT:%u:TR:%u:BA:%u:\
PKTACKED:%u:MSS:%u:BK:%u:FC:%u:SO:%u:ACKN:%u:SNDUNA:%u:SNDNXT:%u:\
LDS:%u:LAR:%u:LDR:%u:\
LE:%u:CWRSE:%u:AS:%u:ANS:%u:FRE:%u:REORD:%u:TCPSTATE:%u",
                        (long long)tp->delta,ipbuf,tp->snd_cwnd ,tp->rcv_wnd,tp->ssthresh, 
                        tp->rtt, tp->rttvar ,tp->srtt, tp->srtt_jtu, tp->rto,
                        tp->in_flight, tp->packets_out, tp->lost_out, tp->retrans_out, tp->prr_delivered,tp->prr_out ,tp->total_retrans,tp->bytes_acked,
                        tp->numpktacked, tp->cachemss, tp->backoff, tp->frto_counter, tp->sacked_out, tp->ack_seq, tp->snd_una, tp->snd_nxt,
                        tp->last_data_sent , tp->last_ack_recv, tp->last_data_recv,
                        tp->tcpEvent.countLOSS_EVENT,tp->tcpEvent.countCWND_RESTART,tp->tcpEvent.countACKSEQ,tp->tcpEvent.countACKNOSEQ,tp->tcpEvent.countFASTRECOVERYENDED, 
                        tp->reordering , tp->tcpCAState);



                    //                                    
                switch(ctrlPrintLocal){
                    case 1:
                           printf("%s\n" , printString);

#ifdef __ANDROID__
                __android_log_print(ANDROID_LOG_DEBUG, "TCPOBSERVATION", "PRINTF %d" , ctrlPrintLocal );
#endif
                        break;
                    case 2:


                        if(firstACK == 1){
                            firstACK = 0;                                    
                            gettimeofday(&tv, &tz);
                            tm = localtime(&tv.tv_sec);                    
                            fprintf(fp , "%d.%02d.%02d.%06ld:IP:0:CWND:0:AW:0:SSHTHR:0:RTT:0:RTTV:0:SRTT:0:SRTTJTU:0:\
RTO:0:FLIGHT:0:PO:0:PL:0:RO:0:PRRDEV:0:PRROUT:0:TR:0:BA:0:\
PKTACKED:0:MSS:0:BK:0:FC:0:SO:0:ACKN:0:SNDUNA:0:SNDNXT:0:\
LDS:0:LAR:0:LDR:0:\
LE:0:CWRSE:0:AS:0:ANS:0:FRE:0:REORD:0:TCPSTATE:" ,tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec );

                        }



                        fprintf(fp , "%s\n" ,printString);
#ifdef __ANDROID__
                        __android_log_print(ANDROID_LOG_DEBUG, "TCPOBSERVATION", "FPRINTF %d" , ctrlPrintLocal );
#endif
                        fflush(fp);
                        break;
                    case 3:
                        fprintf(fp , "%s\n" , printString);
                        printf("%s\n" , printString);
                        fflush(fp);
#ifdef __ANDROID__
                        __android_log_print(ANDROID_LOG_DEBUG, "TCPOBSERVATION", "FPRINTF %d" , ctrlPrintLocal );
#endif
                        break;
                }//switch
             }else{
 #ifdef __ANDROID__
                     __android_log_print(ANDROID_LOG_DEBUG, "TCPOBSERVATION", "IP DOES NOT ALLOW %s" , ipbuf );
 #else
                     printf("IP DOES NOT ALLOW %s\n" , ipbuf);
 #endif    
            
            }//
            
        }
        else
        {  
        
             if(keepRunningTCP == 1){
                 error("ERROR socket NETLINK");
             }else{
                 printf("EXIT TCPOBSERVATION\n");      
                 ctrlLoopLocal = 0;
                 break;
             }
         }
	}
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "TCPOBSERVATION", "CLOSE TCP OBSERVATION");
#endif


    
    fclose(fp);
    close(sock_fd_snd);
    free(nlh_snd);
    free(nlh_rcv);
    free(absolutePathFile);
    printf("CLOSE TCP OBSERVATION\n");
    return 0;
}

