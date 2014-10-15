/*
Cognitive Network programm TCP CONGESTION WINDOW PROGRAM TO RETRIEVE AND MODIFIY 
SOME TCP PARAMETERS
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

/*
 * DURIP project
 * durip.c
 *
 */
//HEADER NECESSARY TO READ/WRITE Congestion windows

//And finally a warning: for very large BDP paths where the TCP window is > 20 MB, 
//you may hit the Linux SACK implementation problem. If Linux has too many packets in flight when it gets a SACK event,
//it takes too long to locate the SACKed packet, and you get a TCP timeout and CWND goes back to 1 packet.
//Restricting the TCP buffer size to about 12 MB seems to avoid this problem, but clearly limits your total throughput. 
//Another solution is to disable SACK. This appears to have been fixed in version 2.6.26.


//This function is called after the first acknowledgment is received and before the congestion control algorithm will be called for the first time.
//If the congestion control algorithm has private data, it should initialize its private date here.
//tcp_no_metrics_save - BOOLEAN
//By default, TCP saves various connection metrics in the route cache
//when the connection closes, so that connections established in the
//near future can use these to set initial conditions.  Usually, this
//increases overall performance, but may sometimes cause performance
//degradation.  If set, TCP will not cache metrics on closing
//connections.

//BOOK TO BUY TCP/IP Architecture, Design and Implementation in Linux

//ECN Explicit congestion Notification 

#include <net/tcp.h>
#include <linux/module.h>	/* Specifically, a module */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <asm/uaccess.h>	/* for copy_from_user */

//HEADER NECESSARY TO CREATE PROC FILE
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <net/net_namespace.h>
//WRITE ON PROC FILE
#include <linux/fs.h>		// for basic filesystem
#include <linux/seq_file.h>	// for sequence files
#include <linux/jiffies.h>	// for jiffies

#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#include <linux/version.h>

#define NETLINK_USER_SND 31
#define NETLINK_USER_RCV 21

struct sock *nl_sk_snd = NULL;
struct sock *nl_sk_rcv = NULL;

static int usr_pid_snd = 0;
static int usr_pid_rcv = 0;

//PROC FILE SECTION

#define MAX_LEN      4096 

#define PROCFS_NAME_OUTPUT "tcp_output_durip"
#define PROCFS_NAME_EVENT "tcp_event_durip"
#define PROCFS_NAME_INPUT_CWND "tcp_input_cwnd"
#define PROCFS_NAME_INPUT_CWND_CLAMP "tcp_input_cwnd_clamp"
#define PROCFS_NAME_RESET_EVENT "tcp_event_reset"

#define OUTPUT_STRING_EVENT "ACKSEQ:%d:ACKNOSEQ:%d:CWNDR:%d:PKTLOST:%d:LOST_EVENT:%d:BYTES:%u:PKTAC:%u:CLAMP:%u\n"

#define OUTPUT_STRING "IP:%pI4:CWND:%07d:AW:%07d:SSHTHRESH:%u:RTT:%u:SRTT:%u:SRTTJTU:%u:\
RTO:%u:FLIGHT:%u:PACKETOUT:%u:PACKETLOST:%u:PACKETRET:%u:\
PRRDEV:%u:PRROUT:%u:TOTRET:%u:BYTESACK:%u:PKTACKED:%u:MSS:%u\n"


static struct proc_dir_entry *Output_Proc_File;
static struct proc_dir_entry *Output_Proc_File_EVENT;
static struct proc_dir_entry *Input_CWND_Proc_File;
static struct proc_dir_entry *Input_CWND_CLAMP_Proc_File;
static struct proc_dir_entry *Input_RESET_EVENT_Proc_File;


//INFO OUTPUT
static char *info_OUTPUT;
static int write_index_OUTPUT;
static int read_index_OUTPUT;

//INFO OUTPUT_EVENT
static char *info_EVENT;
static int read_index_EVENT ; 
static int write_index_EVENT;

//INFO INPUT_CWND

static char *info_INPUT_CWND;
//static int read_index_INPUT_CWND ;
//static int write_index_INPUT_CWND;


//INFOT INPUT_CWND_CLAMP
static char *info_INPUT_CWND_CLAMP;
//static int read_index_INPUT_CWND_CLAMP ;
//static int write_index_INPUT_CWND_CLAMP;

static char *info_INPUT_RESET_EVENT;

static int createOutputFile(void);
static int createOutputFileEVENT(void);
static int createInputFileCWND_CLAMP(void);
static int createInputFileCWND(void);
static int createInputResetEventFile(void);

int init_nl_parameters(void);
//END PROC FILE SECTION


// ktime_t now;
ktime_t prevTimeAck;
//CHECK INPUT FROM PROC FILES
u32 tmp_clamp;
u32 tmp_clamp2=-1;
u32 setClamp=0;


u32 setCWND = 0;
u32 tmp_cwnd;
u32 cwnd_clamp = 0xffffffff/128 ;


u8 firstACK = 1;
static unsigned int bufsize __read_mostly = 4096;
MODULE_PARM_DESC(bufsize, "Log buffer size in packets (4096)");
module_param(bufsize, uint, 0);

static int checkPrint __read_mostly=1;
MODULE_PARM_DESC(checkPrint, "Full log (1=printk,  0=no printk)");
module_param(checkPrint, int, 0);

int count=0;
u32 in_flight_tmp;
s32 tmp_rtt;
u32 tmp_pkts_acked;

u32 netmask;
u32 subnet;


struct tcpEventCollection{    
    u8 countLOSS_EVENT ;
    u8 countCWND_RESTART ;
    u8 countACKSEQ  ;
    u8 countACKNOSEQ ;    
    u8 countFASTRECOVERYENDED;
};

struct tcp_log {
	// struct timespec tv;
    s64 delta;
	__be32	saddr, daddr;
	__be16	sport, dport;		
	u32	snd_cwnd;
    u32 rcv_wnd;        /* Current receiver window              */
	u32	ssthresh;
	u32 rtt;
    u32	srtt;    
    u32	srtt_jtu;
    u32 rto;   
    u32 rttvar;
	u32	in_flight;        /*in_flight = packets_out - left_out + retrans_out*/
	u32 packets_out;    /*  packets_out is SND.NXT-SND.UNA counted in packets.        */
    u32 retrans_out;    /* Retransmitted packets out     retrans_out is number of retransmitted segments*/
	u32 lost_out;       /* Lost packets                 */
    u32 total_retrans;
    u32 bytes_acked;  
    u32 numpktacked;    
    u32 cachemss;
    u32 sacked_out;
    u32 snd_una;        // First byte we want an ack for        oldest unacknowledged sequence number
    u32 snd_nxt;        // Next sequence we send                next sequence number to be sent
    u32 ack_seq;        /* Sequence number ACK'd        */
    u8  frto_counter;   /* Number of new acks after RTO */    
    u8  backoff;    
    u32 last_data_sent ;
    u32 last_data_recv;
    u32 last_ack_recv;
    u8 reordering; /* Packet reordering metric.*/
    u8 tcpCAState;
    u32 prr_delivered;  /* Number of newly delivered packets to  receiver in Recovery. */
    u32 prr_out;        /* total packet sent during recovery*/

    struct tcpEventCollection tcpEvent;

};
/*   left_out is number of segments left network, but not ACKed yet.
 *
 *      left_out = sacked_out + lost_out
 *
 *     sacked_out: Packets, which arrived to receiver out of order
 *         and hence not ACKed. With SACKs this number is simply
 *         amount of SACKed data. Even without SACKs
 *         it is easy to give pretty reliable estimate of this number,
 *         counting duplicate ACKs.
 *
 *       lost_out: Packets lost by network. TCP has no explicit
 *         "loss notification" feedback from network (for now).
 *         It means that this number can be only _guessed_.
 *         Actually, it is the heuristics to predict lossage that
 *         distinguishes different algorithms.
*/


//u8 reordering /* Packet reordering metric.*/
//u8 tcpCAState

// enum tcp_ca_state {
//     TCP_CA_Open = 0,
// #define TCPF_CA_Open    (1<<TCP_CA_Open)
//     TCP_CA_Disorder = 1,
// #define TCPF_CA_Disorder (1<<TCP_CA_Disorder)
//     TCP_CA_CWR = 2,
// #define TCPF_CA_CWR (1<<TCP_CA_CWR)
//     TCP_CA_Recovery = 3,
// #define TCPF_CA_Recovery (1<<TCP_CA_Recovery)
//     TCP_CA_Loss = 4
// #define TCPF_CA_Loss    (1<<TCP_CA_Loss)
// };


typedef struct ipinfo{
  unsigned long int subnet;
  unsigned long int netmask;

}ipinfo;




static struct tcpEventCollectionTmp{
    spinlock_t	lock;
    u8 countLOSS_EVENT ;
    u8 countCWND_RESTART;
    u8 countACKNOSEQ;
    u8 countACKSEQ;
    u8 countFASTRECOVERYENDED;
}tcpEventCollectionTmp;


static struct {
	ktime_t		start;
	spinlock_t	lock;
	u32		read ;
	struct tcp_log	log;    
} tcp_probe2;


//SECTION COUNT EVENT



u32 countPKTLOSS=0;
u32 countPKTACKED=0;
u32 countBYTESACKED=0;




struct sockaddr_in ip4addrSUBNET;
struct sockaddr_in ip4addrNETMASK;


int ctrlMESSAGE = 1;

/*******************************************************************************************/
/*                                                                                         */
/*                                                                                         */
/*                                    BEGIN IMPLEMENTATION                                 */
/*                                                                                         */
/*                                                                                         */
/*******************************************************************************************/



//static struct proc_dir_entry* jif_file;

static void comandSendMsg(struct tcp_log log);



/*******************************************************************************************/
/*                                    SECTION CWN_ALGORITHM                                */
/*******************************************************************************************/

static u32 tcp_durip_ssthresh(struct sock *sk)
{

	return tcp_reno_ssthresh(sk);
}

/*
 * This function sets the lower bound for congestion window.
 
*This function returns the congestion window after a loo -- it is called after the function ssthresh
*/

static u32 tcp_durip_min_cwnd(const struct sock *sk)
{

//	const struct tcp_sock *tp = tcp_sk(sk);
//	struct tcp_cct *tcp_cct = inet_csk_ca(sk);
//	printk(KERN_INFO "tcp_durip_min_cwnd: "
//	       "snd_cwnd = %d\n", tp->snd_cwnd);

	//return tp->snd_cwnd-20;
	return tcp_reno_min_cwnd(sk);
}

/*
 * This function does the new cwnd calculation.E' da capire!!!
*[This function increases congestion window for each acknowledgment]
*
 */


static void tcp_durip_pkts_acked(struct sock *sk, u32 pkts_acked, s32 rtt_us)
{
//	spin_lock(&tcp_probe.lock);
    tmp_rtt = rtt_us;
    tmp_pkts_acked = pkts_acked; 


//	spin_unlock(&tcp_probe.lock);

}

static void tcp_durip_cwnd_event(struct sock *sk, enum tcp_ca_event event){

        struct sock sk2;
        const struct inet_sock *inet;
        u32 subnetComputed ;
        memcpy( &sk2,sk, sizeof (struct sock));
        inet = inet_sk(&sk2);

        subnetComputed = (inet->inet_daddr & netmask);
        if( subnetComputed== subnet){       
                spin_lock(&tcpEventCollectionTmp.lock);
//            const struct inet_sock *inet = inet_sk(&sk2);
        	switch (event) {
//                	//AN ack in sequence is received 
                        case CA_EVENT_FAST_ACK:
//                                tcpEventCollectionTmp.countACKSEQ = tcpEventCollectionTmp.countACKSEQ + 1;
                                tcpEventCollectionTmp.countACKSEQ = 1;
//                                if(checkPrint == 1)
                               // printk("EVENT FAST ACK \n");     
                        break;
// 		// Congestion window recovery is finished
                        case CA_EVENT_COMPLETE_CWR:
//                                tcpEventCollectionTmp.countFASTRECOVERYENDED = tcpEventCollectionTmp.countFASTRECOVERYENDED +1;
                                tcpEventCollectionTmp.countFASTRECOVERYENDED = 1;
//                                if(checkPrint == 1)
                               // printk("EVENT COMPLETE CWR \n");     
                        break;
                        //fast recovery timeout happens

/*                        case CA_EVENT_FRTO:
//                            if(checkPrint == 1)
//                                printk("EVENT FRTO ACK \n");
*/
                        break;
// 		//an ack not in sequence is received
                        case CA_EVENT_SLOW_ACK:
//                                tcpEventCollectionTmp.countACKNOSEQ = tcpEventCollectionTmp.countACKNOSEQ + 1;
                                tcpEventCollectionTmp.countACKNOSEQ = 1;
//                                if(checkPrint == 1)
                               // printk("EVENT SLOW ACK \n");     
                        break;
// 
//		//first transmission when no packet is in flight
                        case CA_EVENT_TX_START:
//                            if(checkPrint == 1)
                               // printk("EVENT TX START \n");     
                        break;
//		//congestion window is restarted			
                        case CA_EVENT_CWND_RESTART:
//                                tcpEventCollectionTmp.countCWND_RESTART = tcpEventCollectionTmp.countCWND_RESTART +1;
                                    tcpEventCollectionTmp.countCWND_RESTART = 1;
//                                if(checkPrint == 1)
                               // printk("EVENT CWND RESTART \n");     
                        break;
//		//retransmission timeout happens
                        case CA_EVENT_LOSS:
//                                tcpEventCollectionTmp.countLOSS_EVENT = tcpEventCollectionTmp.countLOSS_EVENT + 1;
                                    tcpEventCollectionTmp.countLOSS_EVENT = 1;
//                                if(checkPrint == 1)
                               printk("EVENT LOSS \n");     
                        break;

                        default:
                            if(checkPrint == 1)
                                printk("EVENT DEFAULT \n");
                        break;
                }//switch
                spin_unlock(&tcpEventCollectionTmp.lock);
    }
      	
}





static void tcp_durip_init(struct sock *sk)
{
    struct tcp_sock *tp = tcp_sk(sk);
    const struct inet_sock *inet = inet_sk(sk);
    u32 subnetComputed;
    subnetComputed = (inet->inet_daddr & netmask);
     if( subnetComputed== subnet){
        if(setClamp == 1){
            printk("MATTEO SETCLAMP \n");
            tp->snd_cwnd_clamp = tmp_clamp;    
            setClamp = 0 ;
        }else{
            printk("DEFAULT SETCLAMP \n");
            tp->snd_cwnd_clamp = cwnd_clamp;   
        }
    
    
        if(setCWND == 1){
            printk("MATTEO SET CWND ACTUAL%d --- SET %d\n" , tp->snd_cwnd , tmp_cwnd);
            tp->snd_cwnd = tmp_cwnd;
            setCWND = 0 ;
        }
    }
}

 void tcp_cong_avoid_durip(struct tcp_sock *tp, u32 w )
{
          
    if (tp->snd_cwnd_cnt >= w) {
        if (tp->snd_cwnd < cwnd_clamp)
            tp->snd_cwnd++;
        tp->snd_cwnd_cnt = 0;
    } else {
        tp->snd_cwnd_cnt++;
    }
    
}


void tcp_slow_start_durip(struct tcp_sock *tp)
{
    int cnt; /* increase in packets */
    unsigned int delta = 0;
    u32 snd_cwnd;
    /* RFC3465: ABC Slow start
      * Increase only after a full MSS of bytes is acked
      *
      * TCP sender SHOULD increase cwnd by the number of
      * previously unacknowledged bytes ACKed by each incoming
      * acknowledgment, provided the increase is not more than L
      */
   //  if (sysctl_tcp_abc && tp->bytes_acked < tp->mss_cache)
     //       return;
    cnt = tp->snd_cwnd;                     /* exponential increase */
    snd_cwnd = tp->snd_cwnd;
     /* RFC3465: ABC
      * We MAY increase by 2 if discovered delayed ack
      */
   // if (sysctl_tcp_abc > 1 && tp->bytes_acked >= 2*tp->mss_cache)
   //         cnt <<= 1;
    tp->bytes_acked = 0;

    tp->snd_cwnd_cnt += cnt;

    while (tp->snd_cwnd_cnt >= snd_cwnd) {
        tp->snd_cwnd_cnt -= snd_cwnd;
        delta++;
    }

    tp->snd_cwnd = min(snd_cwnd + delta, cwnd_clamp);

    // while (tp->snd_cwnd_cnt >= tp->snd_cwnd) {
    //         tp->snd_cwnd_cnt -= tp->snd_cwnd;
    //          if (tp->snd_cwnd < cwnd_clamp)
    //                  tp->snd_cwnd++;
    //  }
}



    
    
static void tcp_durip_cong_avoid(struct sock *sk, u32 ack, u32 in_flight)
{
    ktime_t now;
    // ktime_t prevTimeAck;
	struct tcp_sock *tp = tcp_sk(sk);        
	const struct inet_sock *inet = inet_sk(sk);

    u32  subnetComputed;
    const struct inet_connection_sock *icsk = inet_csk(sk);
	in_flight_tmp = in_flight;
    // s64 delta;
    spin_lock(&tcp_probe2.lock);
    subnetComputed = (inet->inet_daddr & netmask);
    if( subnetComputed== subnet){
	        //ktime_get_ts — get the monotonic clock in timespec format
            now = ktime_get();
            if(firstACK == 0){
                
                tcp_probe2.log.delta = ktime_to_ns(ktime_sub(now , prevTimeAck));
            }else{
                tcp_probe2.log.delta = ktime_to_ns(now);
                firstACK = 0;
            }
            
            prevTimeAck = now;



            // ktime_get_ts(&tcp_probe2.log.tv);
            tcp_probe2.log.saddr            = inet->inet_saddr;
            tcp_probe2.log.sport            = inet->inet_sport;
            tcp_probe2.log.daddr            = inet->inet_daddr;
            tcp_probe2.log.dport            = inet->inet_dport;            
            tcp_probe2.log.snd_cwnd         = tp->snd_cwnd;            
            tcp_probe2.log.rcv_wnd          = tp->rcv_wnd;
            tcp_probe2.log.ssthresh         = tcp_current_ssthresh(sk);
            tcp_probe2.log.rtt              = tmp_rtt;            
            tcp_probe2.log.srtt             = tp->srtt >> 3;
            tcp_probe2.log.srtt_jtu         = jiffies_to_usecs(tp->srtt)>>3;            
            tcp_probe2.log.rto              = jiffies_to_usecs(icsk->icsk_rto);   
            tcp_probe2.log.rttvar           = jiffies_to_usecs(tp->mdev)>>2;   

            tcp_probe2.log.in_flight        = in_flight_tmp;
            tcp_probe2.log.packets_out      = tp->packets_out;
            tcp_probe2.log.retrans_out      = tp->retrans_out;
            tcp_probe2.log.lost_out         = tp->lost_out;            		    
            

            // if (tp->bytes_acked >= tp->snd_cwnd*tp->mss_cache) {
            //     tp->bytes_acked -= tp->snd_cwnd*tp->mss_cache;
            // }

            tcp_probe2.log.total_retrans    = tp->total_retrans;
            tcp_probe2.log.bytes_acked      = tp->bytes_acked;
            tcp_probe2.log.numpktacked      = tmp_pkts_acked;
            tcp_probe2.log.cachemss         = tp->mss_cache;
         
            tcp_probe2.log.sacked_out       = tp->sacked_out;
         
            tcp_probe2.log.snd_una          = tp->snd_una;        
            tcp_probe2.log.snd_nxt          = tp->snd_nxt;        
            tcp_probe2.log.ack_seq          = ack;     
            tcp_probe2.log.frto_counter     = tp->frto_counter;
            
            tcp_probe2.log.backoff          = icsk->icsk_backoff; 
            tcp_probe2.log.last_data_sent   = jiffies_to_msecs(tp->lsndtime);
            tcp_probe2.log.last_data_recv   = jiffies_to_msecs(icsk->icsk_ack.lrcvtime);
            tcp_probe2.log.last_ack_recv    = jiffies_to_msecs(tp->rcv_tstamp);


            tcp_probe2.log.reordering       = tp->reordering;
            tcp_probe2.log.tcpCAState       = sk->sk_state;

#ifndef __ANDROID__
            tcp_probe2.log.prr_delivered    = tp->prr_delivered;
            tcp_probe2.log.prr_out          = tp->prr_out;
#else
            tcp_probe2.log.prr_delivered    = 0;
            tcp_probe2.log.prr_out          = 0;      
#endif

            spin_lock(&tcpEventCollectionTmp.lock);       
            
            tcp_probe2.log.tcpEvent.countLOSS_EVENT         = tcpEventCollectionTmp.countLOSS_EVENT;
            tcp_probe2.log.tcpEvent.countCWND_RESTART       = tcpEventCollectionTmp.countCWND_RESTART;
            tcp_probe2.log.tcpEvent.countACKNOSEQ           = tcpEventCollectionTmp.countACKNOSEQ;
            tcp_probe2.log.tcpEvent.countACKSEQ             = tcpEventCollectionTmp.countACKSEQ ;
            tcp_probe2.log.tcpEvent.countFASTRECOVERYENDED  = tcpEventCollectionTmp.countFASTRECOVERYENDED ;
         
            tcpEventCollectionTmp.countACKSEQ              = 0;
            tcpEventCollectionTmp.countLOSS_EVENT          = 0;
            tcpEventCollectionTmp.countCWND_RESTART        = 0;
            tcpEventCollectionTmp.countACKNOSEQ            = 0;
            tcpEventCollectionTmp.countFASTRECOVERYENDED   = 0;
    
            spin_unlock(&tcpEventCollectionTmp.lock);  
         
            tcp_probe2.read = 0; 
            comandSendMsg(tcp_probe2.log);
            tmp_clamp2= tp->snd_cwnd_clamp;
            
            
            if(setCWND == 1){

                tp->snd_cwnd = tmp_cwnd;
                printk("SET CWND with: %d\n" , tp->snd_cwnd);
                setCWND = 0 ;
            }

            if(checkPrint == 1){
                    printk(KERN_INFO "ADDRESS: %lld \n", (long long) tcp_probe2.log.delta);
                    // printk(KERN_INFO "ADDRESS: %x %pI4 \n",inet->inet_daddr , &inet->inet_daddr);
                    // printk(KERN_INFO "SUBNET: %x \n",(inet->inet_daddr & NETMASK)  );

            }
                 
            if (tp->snd_cwnd <= tp->snd_ssthresh){
                tcp_slow_start_durip(tp);

            }else{   
                tcp_cong_avoid_durip(tp, tp->snd_cwnd);
            }
        }else{
            if(checkPrint == 1){
                    //printk(KERN_INFO "NO ADDRESS: %x %pI4  \n",inet->inet_daddr  , &inet->inet_daddr);
                    
            }
		if (tp->snd_cwnd <= tp->snd_ssthresh){
			tcp_slow_start(tp);
		}else{
			tcp_reno_cong_avoid(sk, ack, in_flight);
		}
    }

	spin_unlock(&tcp_probe2.lock);

                
}//congestion avoidance


/*
*******************************************************************************************
SECTION SOCKET
*******************************************************************************************
*/


static void comandSendMsg(struct tcp_log log){

    
	//struct nlmsghdr *nlh;
	struct nlmsghdr *nlh_log;
	int pid;
	//struct sk_buff *skb_out;
	struct sk_buff * skb_out_log;
	//int msg_size;
	int msg_size_log;
	//char *msg="SENT FROM KERNEL";
	int res;

	pid = usr_pid_snd;
	

	//msg_size=strlen(msg);
	msg_size_log = sizeof(log);
	
	//skb_out = nlmsg_new(msg_size,0);
	skb_out_log = nlmsg_new(msg_size_log,0);
	//if(!skb_out){
	//	printk(KERN_ERR "Failed to allocate new skb\n");
	//	return;
	//} 

	if(!skb_out_log){
		printk(KERN_ERR "Failed to allocate new skb\n");
		return;
	}
	//nlh=nlmsg_put(skb_out,0,0,NLMSG_DONE,msg_size,0);  
	nlh_log=nlmsg_put(skb_out_log,0,0,NLMSG_DONE,msg_size_log,0);  

	//NETLINK_CB(skb_out).dst_group = 0; // not in mcast group 
	NETLINK_CB(skb_out_log).dst_group = 0; // not in mcast group 

	//strncpy(nlmsg_data(nlh),msg,msg_size);

	memcpy(nlmsg_data(nlh_log),&log,sizeof(log));

	res=nlmsg_unicast(nl_sk_snd,skb_out_log,pid);
	if(res!=0 && ctrlMESSAGE == 1){
		printk(KERN_INFO "Error while sending back to user\n");
        ctrlMESSAGE = 0;
    }
	
	//free(skb_out_log);
	// free(nlh_log);

}

static void hello_snd_msg(struct sk_buff *skb ) {

	struct nlmsghdr *nlh;
	int pid;
//	struct sk_buff *skb_out;
//	int msg_size;
	if(checkPrint == 1){
		printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
	}
//	msg_size=strlen(msg);
//	skb_out = nlmsg_new(msg_size,0);

	printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
	nlh=(struct nlmsghdr*)skb->data;
	pid = nlh->nlmsg_pid; 
	usr_pid_snd = pid;
}


static void hello_rcv_msg(struct sk_buff *skb ) {

	struct nlmsghdr *nlh;
	int pid;
	ipinfo *ipNL;
	printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
	if(checkPrint == 1){
		printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
	}
	nlh=(struct nlmsghdr*)skb->data;
	ipNL = (ipinfo *)nlmsg_data(nlh);
	pid = nlh->nlmsg_pid; 
	usr_pid_rcv = pid;
	printk("SUBNET: %lx\n", ipNL->subnet );
	printk("NETMASK: %lx\n", ipNL->netmask );
    firstACK = 1;
    spin_lock(&tcp_probe2.lock);
	subnet=ipNL->subnet;
	netmask=ipNL->netmask;
    spin_unlock(&tcp_probe2.lock);
}



/*******************************************************************************************/
/*                                    SECTION READ/WRITE PROC FILE                         */
/*******************************************************************************************/

//OUTPUT FILE
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
int proc_reset_event_info( struct file *file, const char __user *buffer, unsigned long count, void *data )
{
    char *page;
    int size = sizeof(int); /* Example, we require to read a buffer in the size of an integer */
    
      
    page = (char *) vmalloc(size);
    if (!page){
        printk("PAGE\n");
        return -ENOMEM;
    }
    /* Copy the data from the user space */
    if (copy_from_user(page, buffer, size)) {
        vfree(page);
        return -EFAULT;
    }
         
    /* Now do something with the data, here we just print it */
    spin_lock(&tcpEventCollectionTmp.lock);
    tcpEventCollectionTmp.countLOSS_EVENT         = 0;
    tcpEventCollectionTmp.countCWND_RESTART       = 0;
    tcpEventCollectionTmp.countACKNOSEQ           = 0;
    tcpEventCollectionTmp.countACKSEQ             = 0;
    tcpEventCollectionTmp.countFASTRECOVERYENDED  = 0;
    
    spin_unlock(&tcpEventCollectionTmp.lock);
    /* Free the allocated memory */
    vfree(page);
    return 0;
}
#else
int proc_reset_event_info (struct file *file,const char *buffer,size_t count,loff_t *offp){


    char * msgoutput2=kmalloc(GFP_KERNEL,10*sizeof(char));
    if (copy_from_user(msgoutput2, buffer, count)) {
        kfree(msgoutput2);
        return -EFAULT;
    }

/* Now do something with the data, here we just print it */
    spin_lock(&tcpEventCollectionTmp.lock);
    tcpEventCollectionTmp.countLOSS_EVENT         = 0;
    tcpEventCollectionTmp.countCWND_RESTART       = 0;
    tcpEventCollectionTmp.countACKNOSEQ           = 0;
    tcpEventCollectionTmp.countACKSEQ             = 0;
    tcpEventCollectionTmp.countFASTRECOVERYENDED  = 0;
    
    spin_unlock(&tcpEventCollectionTmp.lock);
    kfree(msgoutput2);
    return count;
}
#endif



#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)

int proc_output_info( char *page, char **start, off_t off, int count, int *eof, void *data )
{
    int len;
    __be32	daddr;
    if (off > 0)
    {
        *eof = 1;
        return 0;
    }
    if (read_index_OUTPUT >= write_index_OUTPUT)
        read_index_OUTPUT = 0;

    spin_lock(&tcp_probe2.lock);
    if(tcp_probe2.read == 0 )
    {
        daddr = tcp_probe2.log.daddr;
        len=sprintf(page , OUTPUT_STRING,
                                    &daddr,tcp_probe2.log.snd_cwnd ,tcp_probe2.log.rcv_wnd , tcp_probe2.log.ssthresh ,
                            tcp_probe2.log.rtt, tcp_probe2.log.srtt, tcp_probe2.log.srtt_jtu, tcp_probe2.log.rto,
                            tcp_probe2.log.in_flight, tcp_probe2.log.packets_out, tcp_probe2.log.lost_out, tcp_probe2.log.retrans_out,
                            tcp_probe2.log.prr_delivered,tcp_probe2.log.prr_out ,tcp_probe2.log.total_retrans,tcp_probe2.log.bytes_acked, tcp_probe2.log.numpktacked, tcp_probe2.log.cachemss );  

//      len = sprintf(page, "%pI4 %d %d %d \n",
//      &daddr , tcp_probe2.log.snd_cwnd ,  tcp_probe2.log.ssthresh ,tcp_probe2.log.srtt);
		tcp_probe2.read=1;	
        read_index_OUTPUT += len;
	}
    spin_unlock(&tcp_probe2.lock);
    return read_index_OUTPUT;
}
#else
// ( char *page, char **start, off_t off, int count, int *eof, void *data )

int ctrlPROC=0;
char * msgoutput;
int lenOutput ;
int proc_output_info ( struct file *filp,char *buf,size_t count,loff_t *offp ) 
{
    
    __be32  daddr;
        printk("ENTRO  %s\n" ,  __FUNCTION__);
    if(ctrlPROC == 0){
        spin_lock(&tcp_probe2.lock);
        printk("IF  %s\n" ,  __FUNCTION__);
        if(tcp_probe2.read == 0 )
        {
            ctrlPROC = 1;
            daddr = tcp_probe2.log.daddr;
            lenOutput=sprintf(msgoutput , OUTPUT_STRING,
                                &daddr,tcp_probe2.log.snd_cwnd ,tcp_probe2.log.rcv_wnd , tcp_probe2.log.ssthresh ,
                                tcp_probe2.log.rtt, tcp_probe2.log.srtt, tcp_probe2.log.srtt_jtu, tcp_probe2.log.rto,
                                tcp_probe2.log.flight, tcp_probe2.log.packets_out, tcp_probe2.log.lost_out, tcp_probe2.log.retrans_out,
                                tcp_probe2.log.prr_delivered,tcp_probe2.log.prr_out ,tcp_probe2.log.total_retrans,tcp_probe2.log.bytes_acked, tcp_probe2.log.numpktacked, tcp_probe2.log.cachemss );  

            tcp_probe2.read=1;  
            read_index_OUTPUT = lenOutput;
        }
        spin_unlock(&tcp_probe2.lock);
    }


    if(ctrlPROC == 1){
        if(count > read_index_OUTPUT)
            count = read_index_OUTPUT;

        read_index_OUTPUT = read_index_OUTPUT - count;

        if(copy_to_user(buf, msgoutput , count)){
            // kfree(msgoutput);
            return -EFAULT;
        }

        if(count == 0){
            ctrlPROC=0;
            read_index_OUTPUT = lenOutput;
            // kfree(msgoutput);
        }
        return count; 
    }

    return 0;
}

#endif



//EVENT

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
int proc_output_EVENT( char *page2, char **start, off_t off, int count, int *eof, void *data )
{
    int len;
    if (off > 0)
    {
        *eof = 1;
        return 0;
    }
    if (read_index_EVENT >= write_index_EVENT)
        read_index_EVENT = 0;
    
    spin_lock(&tcpEventCollectionTmp.lock);
    
    len = sprintf(page2 ,OUTPUT_STRING_EVENT,
                  tcpEventCollectionTmp.countACKSEQ , tcpEventCollectionTmp.countACKNOSEQ,
                  tcpEventCollectionTmp.countCWND_RESTART, countPKTLOSS , tcpEventCollectionTmp.countLOSS_EVENT , countBYTESACKED ,countPKTACKED, tmp_clamp2 );
    
    read_index_EVENT += len;
	spin_unlock(&tcpEventCollectionTmp.lock);
    return read_index_EVENT;
    
}
#else

int tmpCtrlEvent=0;
char * msgEvent;
int lenEvent;
int proc_output_EVENT( struct file *filp,char *buf,size_t count,loff_t *offp )
{
    
    
    if(tmpCtrlEvent == 0){
        
        spin_lock(&tcpEventCollectionTmp.lock);    
        lenEvent = sprintf(msgEvent ,OUTPUT_STRING_EVENT,
                  tcpEventCollectionTmp.countACKSEQ , tcpEventCollectionTmp.countACKNOSEQ,
                  tcpEventCollectionTmp.countCWND_RESTART, countPKTLOSS , tcpEventCollectionTmp.countLOSS_EVENT , countBYTESACKED ,countPKTACKED, tmp_clamp2 );
        spin_unlock(&tcpEventCollectionTmp.lock);
        read_index_EVENT= lenEvent;
        tmpCtrlEvent=1;
    }

    if(count > read_index_EVENT)
		count = read_index_EVENT;
	read_index_EVENT =  count - read_index_EVENT;
	if(copy_to_user(buf,msgEvent, count)){
		// kfree(msgEvent);
		return -EFAULT;
	}
    	
	
 	if(count == 0){
        tmpCtrlEvent=0;
 		read_index_EVENT = lenEvent;
        // kfree(msgEvent);
    }
    return count;
}

#endif


//CWND_CLAMP
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)

int proc_input_CWND_CLAMP( struct file *file, const char __user *buffer, unsigned long count, void *data )
{
    char *page;
    int size = sizeof(int); /* Example, we require to read a buffer in the size of an integer */
    int my_data = 0;    /* Make sure that the data is in the right size */
    
    
    page = (char *) vmalloc(size);
    if (!page){
        printk("PAGE\n");
       return -ENOMEM;   
    }
    /* Copy the data from the user space */ 
    if (copy_from_user(page, buffer, size)) {
       vfree(page);
       return -EFAULT;
    }

    /* Now do something with the data, here we just print it */
    sscanf( page,"%d",&my_data );
    tmp_clamp = my_data;
    cwnd_clamp =tmp_clamp;
    printk( "User has sent the value of CLAMP: %d\n", cwnd_clamp );
    setClamp = 1;
    /* Free the allocated memory */
    vfree(page); 
    return count;
}

#else
int proc_input_CWND_CLAMP (struct file *file,const char *buffer,size_t count,loff_t *offp){
    

    char *page;
    int my_data = 0;    /* Make sure that the data is in the right size */
    
    page=kmalloc(GFP_KERNEL,10*sizeof(char)); 
    if (!page){
        printk("PAGE\n");
        return -ENOMEM;
    }
    /* Copy the data from the user space */
    if (copy_from_user(page, buffer, count)) {
        kfree(page);
        return -EFAULT;
    }

    /* Now do something with the data, here we just print it */
    sscanf( page,"%d",&my_data );
    tmp_clamp = my_data;
    cwnd_clamp =tmp_clamp;
    printk( "User has sent the value of CLAMP: %d\n", cwnd_clamp );
    setClamp = 1;
    /* Free the allocated memory */
    kfree(page); 
    return count;
}
#endif



#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
int proc_input_CWND( struct file *file, const char __user *buffer, unsigned long count, void *data )
{
    char *page;
    int size = sizeof(int); /* Example, we require to read a buffer in the size of an integer */
    int my_data = 0;    /* Make sure that the data is in the right size */
    
     page = (char *) vmalloc(size);
    if (!page){
        printk("PAGE\n");
        return -ENOMEM;
    }
    /* Copy the data from the user space */
    if (copy_from_user(page, buffer,count)) {
        vfree(page);
        return -EFAULT;
    }
    
    /* Now do something with the data, here we just print it */
    sscanf( page,"%d",&my_data );
    tmp_cwnd = my_data;
    
    printk( "User has sent the value of CWND:%d  \n", my_data );
    setCWND = 1;
    /* Free the allocated memory */
    vfree(page);
    return count;
}


#else


int proc_input_CWND (struct file *file,const char *buffer,size_t count,loff_t *offp){

    char *page;
    int my_data = 0;    /* Make sure that the data is in the right size */
    
    page=kmalloc(GFP_KERNEL,10*sizeof(char)); 
    if (!page){
        printk("PAGE\n");
        return -ENOMEM;
    }
    /* Copy the data from the user space */
    if (copy_from_user(page, buffer, count)) {
        kfree(page);
        return -EFAULT;
    }
    
    /* Now do something with the data, here we just print it */
    sscanf( page,"%d",&my_data );
    tmp_cwnd = my_data;
    
    printk( "User has sent the value of CWND:%d  \n", my_data );
    setCWND = 1;
    /* Free the allocated memory */
    kfree(page);
    return count;

}
#endif 

/****************************************************************
*                                                               *
*            CREATE PROC FILE                                   *
*                                                               *
*****************************************************************/



#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
static int createInputResetEventFile(){

	info_INPUT_RESET_EVENT = (char *)vmalloc( MAX_LEN );
	memset( info_INPUT_RESET_EVENT, 0, MAX_LEN );
    
    Input_RESET_EVENT_Proc_File = create_proc_entry(PROCFS_NAME_RESET_EVENT, 0666, NULL);
	
	if (Input_RESET_EVENT_Proc_File == NULL) {
		remove_proc_entry(PROCFS_NAME_RESET_EVENT, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_NAME_OUTPUT);
		return -ENOMEM;
	}else{
        Input_RESET_EVENT_Proc_File->write_proc= proc_reset_event_info;
		Input_RESET_EVENT_Proc_File->mode     = S_IFREG | S_IRUGO;
		Input_RESET_EVENT_Proc_File->uid      = 0;
		Input_RESET_EVENT_Proc_File->gid      = 0;
		Input_RESET_EVENT_Proc_File->size     = 1024;
		printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME_OUTPUT);	
		return 1;
	}
}

#else
    struct file_operations proc_fopsEventReset = {
        write: proc_reset_event_info
    };

static int createInputResetEventFile(){
    info_INPUT_RESET_EVENT = (char *)vmalloc( MAX_LEN );
    memset( info_INPUT_RESET_EVENT, 0, MAX_LEN );
    Input_RESET_EVENT_Proc_File = proc_create(PROCFS_NAME_RESET_EVENT, 0 , NULL , &proc_fopsEventReset );
    return 1;
}    
#endif



#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)

static int createOutputFile(){

	info_OUTPUT = (char *)vmalloc( MAX_LEN );
	memset( info_OUTPUT, 0, MAX_LEN );
    
	Output_Proc_File = create_proc_entry(PROCFS_NAME_OUTPUT, 0644, NULL);
	
	if (Output_Proc_File == NULL) {
		remove_proc_entry(PROCFS_NAME_OUTPUT, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_NAME_OUTPUT);
		return -ENOMEM;
	}else{

		write_index_OUTPUT = 0;
        read_index_OUTPUT = 0;
        
        Output_Proc_File->read_proc = proc_output_info;
		Output_Proc_File->mode     = S_IFREG | S_IRUGO;
		Output_Proc_File->uid      = 0;
		Output_Proc_File->gid      = 0;
		Output_Proc_File->size     = 1024;
		printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME_OUTPUT);	
		return 1;
	}
}
#else

struct file_operations proc_fopsOutput = {
        read:   proc_output_info
};

static int createOutputFile(){
        
        msgoutput = kmalloc(512*sizeof(char), GFP_KERNEL);
        
        Output_Proc_File = proc_create(PROCFS_NAME_OUTPUT, 0 , NULL , &proc_fopsOutput);
        if (Output_Proc_File == NULL) {
            printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_NAME_OUTPUT);
            return -ENOMEM;
        }
    return 1;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
static int createOutputFileEVENT()
{

	info_EVENT = (char *)vmalloc( MAX_LEN );
	memset( info_EVENT, 0, MAX_LEN );
	Output_Proc_File_EVENT = create_proc_entry(PROCFS_NAME_EVENT, 0666, NULL);
	if (Output_Proc_File_EVENT == NULL) {
		remove_proc_entry(PROCFS_NAME_EVENT, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_NAME_EVENT);
		return -ENOMEM;
	}else{

		write_index_EVENT = 0;
        read_index_EVENT = 0;
        Output_Proc_File_EVENT->read_proc = proc_output_EVENT;

		Output_Proc_File_EVENT->mode 	  = S_IFREG | S_IRUGO;
		Output_Proc_File_EVENT->uid 	  = 0;
		Output_Proc_File_EVENT->gid 	  = 0;
		Output_Proc_File_EVENT->size 	  = 1024;
		printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME_EVENT);	
		return 1;
	}
}
#else

    struct file_operations proc_fopsEvent = {
        read:   proc_output_EVENT        
    };

static int createOutputFileEVENT()
{    

    msgEvent=kmalloc(128*sizeof(char), GFP_KERNEL);
    info_EVENT = (char *)vmalloc( MAX_LEN );    
    memset( info_EVENT, 0, MAX_LEN );

    Output_Proc_File_EVENT = proc_create(PROCFS_NAME_EVENT, 0 , NULL , &proc_fopsEvent);
    write_index_EVENT = 0;
    read_index_EVENT = 0;
    return 1;
}
#endif

/*
INPUT CWND CLAMP
*/

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
static int createInputFileCWND_CLAMP(){
    
	info_INPUT_CWND_CLAMP = (char *)vmalloc( MAX_LEN );
	memset( info_INPUT_CWND_CLAMP, 0, MAX_LEN );
    
	Input_CWND_CLAMP_Proc_File = create_proc_entry(PROCFS_NAME_INPUT_CWND_CLAMP,0666 , NULL);
	
	if (Input_CWND_CLAMP_Proc_File == NULL) {
		remove_proc_entry(PROCFS_NAME_INPUT_CWND_CLAMP, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
               PROCFS_NAME_INPUT_CWND_CLAMP);
		return -ENOMEM;
	}else{
        
	    Input_CWND_CLAMP_Proc_File->write_proc = proc_input_CWND_CLAMP;
		Input_CWND_CLAMP_Proc_File->uid 	  = 0;
		Input_CWND_CLAMP_Proc_File->gid 	  = 0;
		Input_CWND_CLAMP_Proc_File->size 	  = 1024;
		printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME_INPUT_CWND_CLAMP);
		return 1;
	}
}
#else
    struct file_operations proc_fops_CWND_CLAMP = {
       write: proc_input_CWND_CLAMP    
    };

static int createInputFileCWND_CLAMP(){
   
    
    info_INPUT_CWND_CLAMP = (char *)vmalloc( MAX_LEN );
    memset( info_INPUT_CWND_CLAMP, 0, MAX_LEN );

    Input_CWND_CLAMP_Proc_File = proc_create(PROCFS_NAME_INPUT_CWND_CLAMP, 0 , NULL , &proc_fops_CWND_CLAMP);
    return 1;
    
}
#endif


/*
Input CWND 
*/

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
static int createInputFileCWND(){
    
	info_INPUT_CWND = (char *)vmalloc( MAX_LEN );
	memset( info_INPUT_CWND, 0, MAX_LEN );
	Input_CWND_Proc_File = create_proc_entry(PROCFS_NAME_INPUT_CWND, 0666, NULL);


	if (Input_CWND_Proc_File == NULL) {
		remove_proc_entry(PROCFS_NAME_INPUT_CWND, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
               PROCFS_NAME_EVENT);
		return -ENOMEM;
	}else{
        
        Input_CWND_Proc_File->write_proc = proc_input_CWND;
        
//		Input_CWND_Proc_File->mode 	  = S_IFREG | S_IRUGO;
		Input_CWND_Proc_File->uid 	  = 0;
		Input_CWND_Proc_File->gid 	  = 0;
		Input_CWND_Proc_File->size 	  = 1024;
		printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME_INPUT_CWND);
		return 1;
	}
}
#else 
    struct file_operations proc_fops_CWND = {        
        write: proc_input_CWND        
    };

static int createInputFileCWND(){
    
    info_INPUT_CWND = (char *)vmalloc( MAX_LEN );
    memset( info_INPUT_CWND, 0, MAX_LEN );

    Input_CWND_Proc_File = proc_create(PROCFS_NAME_INPUT_CWND, 0 , NULL , &proc_fops_CWND);
    return 1;

}
#endif

//CREATE DIR DURIP

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
struct proc_dir_entry * make_proc_dir( char *name )
{
   struct proc_dir_entry *my_proc_dir = NULL;     
   /* Create a new directory */
   my_proc_dir = create_proc_entry( name, S_IFDIR, NULL );     

   if (my_proc_dir)
   {
      /* No Read and Write functions here */
      my_proc_dir->read_proc  = NULL;
      my_proc_dir->write_proc = NULL;
   }   
   return my_proc_dir;


}

#else 

struct proc_dir_entry * make_proc_dir( char *name )
{

   struct proc_dir_entry *my_proc_dir = NULL;     
   my_proc_dir = proc_create(name, S_IFDIR , NULL , NULL);
   return my_proc_dir;
}
#endif
/*******************************************************************************************/
/*                                    SECTION INITIALIZATION                               */
/*******************************************************************************************/




static struct tcp_congestion_ops tcp_durip = {
	.ssthresh	= tcp_durip_ssthresh,
	.cong_avoid	= tcp_durip_cong_avoid,
	.min_cwnd	= tcp_durip_min_cwnd,
	.pkts_acked	= tcp_durip_pkts_acked,
	//PARTE NUOVA
    .init = tcp_durip_init,
    
	.cwnd_event	= tcp_durip_cwnd_event,

	.owner		= THIS_MODULE,
	.name		= "DURIP",
};



int init_nl_parameters(){
    
    
//#if LINUX_VERSION_CODE > KERNEL_VERSION(3,8,0)    
//    struct netlink_kernel_cfg cfg;
//    struct netlink_kernel_cfg cfg2;
//#endif
    
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
    nl_sk_snd=netlink_kernel_create(&init_net, NETLINK_USER_SND, 0, hello_snd_msg,NULL, THIS_MODULE);
    if(!nl_sk_snd){
        printk(KERN_ALERT "Error creating socket SND.\n");
        return -10;
    }
    nl_sk_rcv=netlink_kernel_create(&init_net, NETLINK_USER_RCV, 0, hello_rcv_msg,NULL, THIS_MODULE);
    if(!nl_sk_rcv){
        printk(KERN_ALERT "Error creating socket RCV.\n");
        return -10;
    }
#else
    struct netlink_kernel_cfg cfg = {
        .input = hello_snd_msg,
    };
    struct netlink_kernel_cfg cfg2 = {
        .input = hello_rcv_msg,
    };

    nl_sk_snd = netlink_kernel_create(&init_net, NETLINK_USER_SND, &cfg);    
    nl_sk_rcv = netlink_kernel_create(&init_net, NETLINK_USER_RCV, &cfg2);
    
    printk(KERN_ALERT "INIT NL.\n");
#endif
 
return 0;

}


static int __init tcp_durip_register(void)
{
    printk("********************\n");
    printk("*LOAD COGNET MODULE*\n");
    printk("********************\n");
    netmask = 0x00000000;
    subnet = 0xffffffff;
    cwnd_clamp = 1000;  
    firstACK = 1;
    
    printk(KERN_INFO "TCP CLAMP %d \n" , cwnd_clamp);
    // make_proc_dir("DURIP");
	if(createOutputFile()==0){
	   printk(KERN_INFO "ERROR %s" , __FUNCTION__);
   	return -1;
    }
    // if(createOutputFileEVENT() ==0)
    //     return -1;
     if(createInputFileCWND_CLAMP()==0)
         return -1;
     if(createInputFileCWND()==0)
         return -1;
     // if(createInputResetEventFile() == 0)
     //     return -1;
    
    // now = ktime_get();

    spin_lock_init(&tcp_probe2.lock);        
    spin_lock_init(&tcpEventCollectionTmp.lock);
		
    tcp_probe2.start = ktime_get();
    tcp_probe2.read = 1;			   
    
    tcp_probe2.log.tcpEvent.countLOSS_EVENT         = 0;
    tcp_probe2.log.tcpEvent.countCWND_RESTART       = 0;
    tcp_probe2.log.tcpEvent.countACKNOSEQ           = 0;
    tcp_probe2.log.tcpEvent.countACKSEQ             = 0;
    tcp_probe2.log.tcpEvent.countFASTRECOVERYENDED  = 0;
     
    tcpEventCollectionTmp.countLOSS_EVENT         = 0;
    tcpEventCollectionTmp.countCWND_RESTART       = 0;
    tcpEventCollectionTmp.countACKNOSEQ           = 0;
    tcpEventCollectionTmp.countACKSEQ             = 0;
    tcpEventCollectionTmp.countFASTRECOVERYENDED  = 0;
                

    init_nl_parameters();

    
    return tcp_register_congestion_control(&tcp_durip);	
	
	
}


/*******************************************************************************************/
/*                                    SECTION UNREGISTER                                   */
/*******************************************************************************************/

static void __exit tcp_durip_unregister(void)
{


    remove_proc_entry(PROCFS_NAME_OUTPUT, NULL);
    remove_proc_entry(PROCFS_NAME_EVENT, NULL);
    remove_proc_entry(PROCFS_NAME_INPUT_CWND_CLAMP, NULL);
    remove_proc_entry(PROCFS_NAME_INPUT_CWND, NULL);
    remove_proc_entry(PROCFS_NAME_RESET_EVENT, NULL);
    
	

    printk("**********************\n");
    printk("*UNLOAD COGNET MODULE*\n");
    printk("**********************\n"); 
    
	tcp_unregister_congestion_control(&tcp_durip);
	
	netlink_kernel_release(nl_sk_snd);
	netlink_kernel_release(nl_sk_rcv);
    
	vfree(info_OUTPUT);
    vfree(info_EVENT);
    vfree(info_INPUT_CWND);
    vfree(info_INPUT_CWND_CLAMP);
    
    
    
}



//SECTION MODULE
module_init(tcp_durip_register);
module_exit(tcp_durip_unregister);

MODULE_AUTHOR("Matteo Danieletto");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DURIP PROJECT ER MEJO ;)");
