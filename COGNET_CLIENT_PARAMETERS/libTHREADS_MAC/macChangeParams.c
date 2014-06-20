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
#include <string.h>
#include <unistd.h>
//#include <time.h>
#include <linux/wireless.h>

//#define IW_NAME "wlan1"
#include "../include/macChangeParams.h"



float dbm2mw(float in){
    return((float)pow(10.0, in / 10.0));
}

float mw2dbm(float in){
    return((float)pow(10.0, in / 10.0));
}





/****************************************************************************************
 *			GET TRANSMISTION POWER                                                          *
 *****************************************************************************************/


int getSensivity(char * name){
    
    int sockfd;
        struct iw_statistics stats;
	struct iwreq req = {
//		.ifr_name = IW_NAME,
		.u.data = {
			.pointer = &stats,
#ifdef CLEAR_UPDATED
			.flags = 1
#endif
		}
	};
        strcpy(req.ifr_ifrn.ifrn_name, name);
        req.u.data.length = sizeof(stats);
        if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("Could not create simple datagram socket");
		exit(EXIT_FAILURE);
	}
        printf("PROVA\n");
   
        if (ioctl(sockfd, SIOCGIWSENS, &req) >= 0) {
		printf("SENSIVITY: %d\n" ,  req.u.sens.value );      	
	//            wlinfo->sensitivity = req.u.sens.value;
        }
        return 0;
}


int getTXpower(char *name){
//int getTXpower(){
        int power;
        int sockfd;
        struct iw_statistics stats;
	struct iwreq req = {
		//.ifr_name = IW_NAME,
            
//            strcpy(.ifr_name, name);
//                .ifr_name = {name},
		.u.data = {
			.pointer = &stats,
#ifdef CLEAR_UPDATED
			.flags = 1
#endif
                }
		
	};
        
        
        strcpy(req.ifr_ifrn.ifrn_name, name);
        req.u.data.length = sizeof(stats);
        if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("Could not create simple datagram socket");
		exit(EXIT_FAILURE);
	}

        if (ioctl(sockfd, SIOCGIWTXPOW, &req) >= 0) {
                if (req.u.txpower.flags == IW_TXPOW_DBM) {
                      //  printf("%f POWER dbm\n" ,(float) req.u.txpower.value );
                        power = req.u.txpower.value ; 
                        close(sockfd);
                        return power;
                }else{
                        printf("%f POWER dbm\n" , mw2dbm(req.u.txpower.value));

                }
        }
        perror("IOTCL\n");
        close(sockfd);
        return -1; 
}


//int getTXchannel(char *name){
int getTXchannel(char *name){
    return 0;
}

//int getTXfrequency(char *name){
int getTXfrequency(char *name){
        int sockfd;
        struct iw_statistics stats;
        struct iwreq req = {
//		.ifr_name = IW_NAME,
		.u.data = {
			.pointer = &stats,
#ifdef CLEAR_UPDATED
			.flags = 1
#endif
		}
	};
    
	req.u.data.length = sizeof(stats);
    strcpy(req.ifr_ifrn.ifrn_name, name);
	/* Any old socket will do, and a datagram socket is pretty cheap */
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("Could not create simple datagram socket");
		exit(EXIT_FAILURE);
	}    
    /* Frequency */
    
        if (ioctl(sockfd, SIOCGIWFREQ, &req) >= 0) {
                //if e==1, therefore the exponential is equal to 1 
                if (req.u.freq.e){
//                      float freq = (req.u.freq.m * pow(10, req.u.freq.e)) / GIGA;
//                      printf("FREQUENCY:%d \n" , req.u.freq.m);
                    int freq = req.u.freq.m;
                    close(sockfd);
                    return freq;
                }

        
        }
        
        perror("IOTCL\n");
        close(sockfd);
        return -1;    
}


/****************************************************************************************
*			SET TRANSMISTION POWER						*
*****************************************************************************************/

int setTXpower(char *name, int new_txpower){


	int sockfd;
	struct iw_statistics stats;
	struct iwreq req = {
//		.ifr_name = IW_NAME,
		.u.data = {
			.pointer = &stats,
	#ifdef CLEAR_UPDATED
			.flags = 1
	#endif
		}
	};

	/* Any old socket will do, and a datagram socket is pretty cheap */
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("Could not create simple datagram socket");
		exit(EXIT_FAILURE);
	}
	req.u.data.length = sizeof(stats);
    strcpy(req.ifr_ifrn.ifrn_name, name);
        if (ioctl(sockfd, SIOCGIWTXPOW, &req) >= 0) {
                req.u.txpower.value = new_txpower;
                if (ioctl(sockfd, SIOCSIWTXPOW, &req) >= 0) {
                        close(sockfd);
                        return 0;      
                }else
                    perror("IOCTL\n");
        }   
        
	close(sockfd);
	return -1;
}


int setTXchannel(char *name, int channel){
	int freq =(float) ((OFFSET_CHANNEL+((channel -1) * STEP_CHANNEL)));
	//printf("CHANNEL:%f\n", freq);	  
    return setTXfrequency(name ,freq);

}


int setTXfrequency(char *name, int freq){

	int sockfd;
	struct iw_statistics stats;
	struct iwreq req = {
//		.ifr_name = IW_NAME,
		.u.data = {
			.pointer = &stats,
#ifdef CLEAR_UPDATED
			.flags = 1
#endif
		}
	};
    strcpy(req.ifr_ifrn.ifrn_name, name);
	req.u.data.length = sizeof(stats);
	/* Any old socket will do, and a datagram socket is pretty cheap */
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("Could not create simple datagram socket");
		exit(EXIT_FAILURE);
	}


        req.u.freq.m = freq * 100000 ;
	printf("Value:%i\n" , freq * 100000);
        req.u.freq.e=1;
        if (ioctl(sockfd, SIOCSIWFREQ, &req) >= 0) {
                close(sockfd);
                return 0;            
        }else{
            perror("IOTCL\n");
    }
	close(sockfd);
	return -1;
}
