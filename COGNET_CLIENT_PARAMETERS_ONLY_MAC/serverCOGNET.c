/*
Cognitive Network programm main file
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>
//BEEP HEADER
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include <arpa/inet.h>

#include <signal.h>

// #include <sys/time.h>

// #ifndef CLOCK_TICK_RATE
// #define CLOCK_TICK_RATE 1193180
// #endif
#include "./include/controlThreadComm.h"
#include "./include/observation.h"
#include "./include/controlREADDURIP_Thread.h"
#include "./include/commonFunction.h"



#ifdef __ANDROID__
#include <android/log.h>
#endif




//TO ADD A GLOBAL VARIABLE TO STORE THE FOLDER NAME




//VARIABILI GLOBALI PER LA GESTIONE DEI THREAD
int ctrlLoopGlobal = 1;
int ctrlPrintGlobal;
float ctrlTSGlobal;


char folder[32];

//int ctrlLoopTmp = 1;
pthread_mutex_t lock;

//GLOBAL VARIABLES OF COMMUNICATION BETWEEN C and APP
pthread_mutex_t lock_comm;
int socketCommApp = -1;
char stationToPlot[18];
//END

static int keepRunning = 1;

int sockfdMain;
int sockfdCommDurip;

int manageThreadsAPP(int sock, int argc, char *argv[]);

void  closeAPP(int pp);

void  closeAPP(int pp) {
    // keepRunning = 0;
    close(sockfdMain);    
    printf("\n----------------------------\n");
    printf("CLOSE COGNET APP\n");
    printf("----------------------------\n");
    //return 0;
}





//RICORDASI DI DEFINIRE QUA UNA VARIABILE DAL COMPILATORE!!
#ifdef __ANDROID__
  #if ANDROID_EXE == 1
    int main(int argc, char *argv[])
  #else
    int mainReadMacServer(int argc, char *argv[])
  #endif

#else
int main(int argc, char *argv[])
#endif
{
    int newsockfdMain, portno,  ii;
    // int pid ;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char **argv2;
#ifdef __ANDROID__

    __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "PORT:%s", argv[0]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "PRINT: %s", argv[1]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "TIME: %s", argv[2]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "PHY: %s", argv[3]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "WLAN: %s", argv[4]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "FOLDER LOG:%s", argv[5]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "IP SUBNET: %s", argv[6]);
    __android_log_print(ANDROID_LOG_DEBUG, "MACREADDURIP", "NETMASK: %s", argv[7]);    
#endif 



#ifdef __ANDROID__ 
#if ANDROID_EXE == 0
  portno = atoi(argv[0]);
    #else

    if (argc < 8) {
        printf("<Socket port><[1-3]>:<time in Seconds><phy{X}><wlan{Y}>\n 1-print only on video 2- print only on file 3 print on video and file\n");
        return -1;
    }

    signal(SIGINT, closeAPP);
    printf("NAME FOLDER:%s\n", argv[6]);     

    argv2 = (char **) calloc((argc-1),sizeof (char *));
    //SOCKET PORT
    argv2[0] = (char*) calloc(strlen(argv[1])+1,sizeof (char));
    strcpy(argv2[0], argv[1]);
    //PRINT OR SAVE LOG FILES
    argv2[1] = (char*) calloc(strlen(argv[2])+1,sizeof (char));
    strcpy(argv2[1], argv[2]);
    //TIMESAMPLE [MS]
    argv2[2] = (char*) calloc(strlen(argv[3])+1,sizeof (char));
    strcpy(argv2[2], argv[3]);
    //PHYX DEVICE
    argv2[3] = (char*) calloc(strlen(argv[4])+1,sizeof (char));
    strcpy(argv2[3], argv[4]);
    //WLANY DEVICE
    argv2[4] = (char*) calloc(strlen(argv[5])+1,sizeof (char));
    strcpy(argv2[4], argv[5]);
    //FOLDER NAME WHERE TO SAVE LOG FILES
    argv2[5] = (char*) calloc(strlen(argv[6])+1,sizeof (char));
    strcpy(argv2[5], argv[6] );
    
    argv2[6] = (char*) calloc(strlen(argv[7])+1,sizeof (char));
    strcpy(argv2[6], argv[7] );

    argv2[7] = (char*) calloc(strlen(argv[8])+1,sizeof (char));
    strcpy(argv2[7], argv[8] );
    
    portno = atoi(argv2[0]);


#endif




#else
    if (argc < 8) {
        printf("<Socket port><[1-3]>:<time in Seconds><phy{X}><wlan{Y}>\n 1-print only on video 2- print only on file 3 print on video and file\n");
        return -1;
    }

    signal(SIGINT, closeAPP);
    printf("NAME FOLDER:%s\n", argv[6]);     

    argv2 = (char **) calloc((argc-1),sizeof (char *));
    //SOCKET PORT
    argv2[0] = (char*) calloc(strlen(argv[1])+1,sizeof (char));
    strcpy(argv2[0], argv[1]);
    //PRINT OR SAVE LOG FILES
    argv2[1] = (char*) calloc(strlen(argv[2])+1,sizeof (char));
    strcpy(argv2[1], argv[2]);
    //TIMESAMPLE [MS]
    argv2[2] = (char*) calloc(strlen(argv[3])+1,sizeof (char));
    strcpy(argv2[2], argv[3]);
    //PHYX DEVICE
    argv2[3] = (char*) calloc(strlen(argv[4])+1,sizeof (char));
    strcpy(argv2[3], argv[4]);
    //WLANY DEVICE
    argv2[4] = (char*) calloc(strlen(argv[5])+1,sizeof (char));
    strcpy(argv2[4], argv[5]);
    //FOLDER NAME WHERE TO SAVE LOG FILES
    argv2[5] = (char*) calloc(strlen(argv[6])+1,sizeof (char));
    strcpy(argv2[5], argv[6] );
    
    argv2[6] = (char*) calloc(strlen(argv[7])+1,sizeof (char));
    strcpy(argv2[6], argv[7] );

    argv2[7] = (char*) calloc(strlen(argv[8])+1,sizeof (char));
    strcpy(argv2[7], argv[8] );
    
    portno = atoi(argv2[0]);
#endif



    
#ifdef __ANDROID__
#if ANDROID_EXE == 0
    checkParameterAppAndroid(argv);
    functionCreateFolder(argv[5]);
    #else
    functionCreateFolder(argv2[5]);
    #endif
#else
    functionCreateFolder(argv2[5]);
#endif

    sockfdMain = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfdMain < 0){
        error("ERROR opening socket");
    }
    bzero((char *) &serv_addr, sizeof (serv_addr));

    

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfdMain, (struct sockaddr *) &serv_addr,
            sizeof (serv_addr)) < 0){
        error("ERROR on binding");
        
    }
    listen(sockfdMain, 5);
    clilen = sizeof (cli_addr);

    // folder= malloc(32 * sizeof(char));
    
    while (keepRunning) {

      if(*folder){                   

#ifdef __ANDROID__ 
#if ANDROID_EXE == 0
  
        argv[5]= (char *)realloc(argv[5],(strlen(folder)+1)*sizeof (char));
        strcpy(argv[5], folder );          
        functionCreateFolder(argv[5]);
#else
      argv2[5] = (char *)realloc(argv2[5],(strlen(folder)+1)*sizeof (char));
      strcpy(argv2[5], folder );
      functionCreateFolder(argv2[5]);
      
#endif   
#else
      argv2[5] = (char *)realloc(argv2[5],(strlen(folder)+1)*sizeof (char));
      strcpy(argv2[5], folder );
      functionCreateFolder(argv2[5]);
      
#endif
      }
    




        ctrlLoopGlobal = 1;
        newsockfdMain = accept(sockfdMain, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfdMain < 0){
          break;
        }else{
          // pid = fork();
          // if (pid < 0)
          //     error("ERROR on fork");

          // if (pid == 0) {
          //     //CHILD PROCESS
          //     close(sockfdMain);

              
              // else{
              //   printf("FOLDER %s\n" , folder);             
              // }

#ifdef __ANDROID__
          #if ANDROID_EXE == 0
              manageThreadsAPP(newsockfdMain, argc, argv);
            #else
manageThreadsAPP(newsockfdMain, argc, argv2);              
          #endif

#else

              manageThreadsAPP(newsockfdMain, argc, argv2);

#endif        
              //RETURN VALUE  
          //     exit(0);
          // } else {
              close(newsockfdMain);
          }
      // }
    } /* end of while */



#ifdef __ANDROID__
    for(ii=0;ii<argc-1;ii++)
      free(argv[ii]);
    free(argv);
#else
    for(ii=0;ii<argc-1;ii++)
      free(argv2[ii]);
    free(argv2);
#endif


    // close(sockfdMain);
    //     #endif
    return 0; /* we never get here */
}



/*
*/

int manageThreadsAPP(int sock, int argc, char *argv[]) {

  
  pthread_t handleReadDURIP_v;
  //pthread_t threadOBSERVETCP;
  pthread_t threadOBSERVEMAC;
  pthread_t handleCommDURIP_v;
  

  int n;
  char buffer[256];
  char s[8];
  bzero(buffer, 256);
  char ifname[6];
  char * test;
  int second;


  struct sockaddr_in ip4addr;
  struct sockaddr_in ip4addrMASK;
  
  paramThread_Comm param_comm;
  paramThread param;
  paramThread_MACREAD paramMacObser;
 // paramThread_tcp  paramThreadTCP;

  inet_pton(AF_INET, argv[6], &ip4addr.sin_addr);
  inet_pton(AF_INET, argv[7], &ip4addrMASK.sin_addr);

//  paramThreadTCP.ipNL.subnet = ip4addr.sin_addr.s_addr;
//  paramThreadTCP.ipNL.netmask = ip4addrMASK.sin_addr.s_addr;
//  paramThreadTCP.nameExperiment =  (char *)malloc(sizeof(char) *(strlen(argv[5])+1));

//  strcpy(paramThreadTCP.nameExperiment , argv[5]);

  n = read(sock, buffer, 255);
  if (n < 0) {
      error("ERROR reading from socket");
      return -1;
  }
  // printf("Value n: %d \n", n);              
  test = malloc((sizeof (char)+1) * n);
  memcpy(test, buffer, n);
  printf("Message received: %s \n", buffer);
  second = atoi(test);
  free(test);      
  findTime(s);
  n = write(sock, s, 8);

  if (n < 0) {
      error("ERROR writing to socket");
      return -1;
  }

  paramMacObser.argc = argc;
  paramMacObser.argv = argv;

   

  ctrlPrintGlobal =atoi(argv[1]);
  ctrlTSGlobal =atoi(argv[2]);
  
  
  printf("SERVER:%s ---- %d\n", argv[4] , (int)strlen(argv[4]));         
  //PARAMETRIZZARE QUESTO VALORE
  n = write(sock, "START", strlen("START"));
    
  strcpy(ifname, argv[4]);
  strcpy(param.port, argv[0]);
  strcpy(param.ifname, ifname);
    
  if (pthread_mutex_init(&lock, NULL) != 0) {
      printf("\n mutex init failed\n");
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "SERVERREADDURIP", "mutex init failed");
#endif
      return -1;
  }

  if (pthread_mutex_init(&lock_comm, NULL) != 0)
  {
        printf("\n mutex init failed\n");
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "mutex init failed");
#endif        
        return -1;
  }
    
  
  strcpy(param_comm.port, argv[0]);
  strcpy(param_comm.ifname, ifname);


  if (pthread_create(&handleReadDURIP_v, NULL, handleReadDURIP, &param) != 0) {
#ifdef __ANDROID__
       __android_log_print(ANDROID_LOG_DEBUG, "SERVERREADDURIP", "ERROR HANDLE MAC READ DURIP");
#endif
      printf("Error to create thread\n");
      return -2;
  }

  //SECTION FOR COMMUNICATIN BETWEEN APP AND MACOBSERVATION
  if (pthread_create(&handleCommDURIP_v, NULL, handleCommDURIP, &param_comm) != 0){
  #ifdef __ANDROID__
          __android_log_print(ANDROID_LOG_DEBUG, "MACOBSERVATION", "ERROR HANDLE MAC READ DURIP");
  #else
          printf("Error to create thread\n");                
  #endif

        return -1;
  }

/*

*/
  sleep(second);

/*
THREAD OBSERVATION
*/

//  if (pthread_create(&threadOBSERVETCP, NULL, tcpObservation, &paramThreadTCP) != 0) {
//      printf("Error to create thread\n");
//      return -2;
//  }

    
  if (pthread_create(&threadOBSERVEMAC, NULL, macObservation, &paramMacObser) != 0) {
      printf("Error to create thread\n");
      return -2;
  }


/*
CLOSE THREAD
*/

  if (pthread_join(threadOBSERVEMAC , NULL)) {
      fprintf(stderr, "Error joining thread\n");
      return -4;
  }else{
    printf("MAC THREAD DOWN\n");
  }  

  // strcpy(folderLocal , folder);
  // printf("FOLDER LOCAL %s\n" , folderLocal);

  if (pthread_join(handleReadDURIP_v , NULL)) {
      fprintf(stderr, "Error joining thread READ\n");
      return -4;
  }else{
    printf("HANLDE DURIP DOWN\n");
  }  

  
  shutdown(sockfdCommDurip,SHUT_RDWR);
  if (pthread_join(handleCommDURIP_v , NULL)) {
      fprintf(stderr, "Error joining thread HANDLE COMM\n");
      return -4;
  }else{
    printf("HANDLE COMM DOWN\n");
  }  

//  printf("BEFORE TO DROP DOWN TCP THREAD\n");
  
  
//  pthread_kill(threadOBSERVETCP, SIGTERM);
  // shutdown(sock_fd_rcv,SHUT_RDWR);
//  if (pthread_join(threadOBSERVETCP , NULL)) {
//      fprintf(stderr, "Error joining thread TCP\n");
//      return -4;
//  }else{
//    printf("TCP DOWN\n");    
//  }
  
  printf("END %s\n" , __FUNCTION__);
  pthread_mutex_destroy(&lock);
  pthread_mutex_destroy(&lock_comm);
#ifdef __ANDROID__
    
#else
//    if (system("cp -r --no-clobber /mnt/local/log/ /home/danielet/Dropbox/COGNET/`uname -n`/")==-1)
      //  error("SYSTEM CALL ERROR");
            
#endif
  // printf("CLOSE FORK\n");  
  // close(sock);
  return 0;
}
