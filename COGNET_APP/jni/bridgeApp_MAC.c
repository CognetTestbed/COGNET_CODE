#include <jni.h>
#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "it_durip_app_ServerSocketCmd.h"
#include "serverThread.h"

int ctrlFirst = 1;

JNIEXPORT jint JNICALL Java_it_durip_1app_ServerSocketCmd_mainJNI(JNIEnv *env, jobject thisObj ,
		jint n, jobjectArray stringArray){

	int ii;
	    char scriptToLaunch[100];
	    char IP_ADDR[15];
	    char IP_SUBNET[15];
	    char IP_NETMASK[15];
	    char WLAN[10];
	    char PHY[10];
	    char PORT[10];
	    char ESSID[25];
	    char **argv;
	    int stringCount = (*env)->GetArrayLength(env, stringArray);

	    argv = (char **) malloc(sizeof( char *) * stringCount);

	    for(ii=0; ii < stringCount; ii++) {
	        jstring element = (*env)->GetObjectArrayElement(env, stringArray, ii);
	        const char * rawstring = (*env)->GetStringUTFChars(env, element, NULL);
	        switch (ii) {
	            case 0:
	                strcpy(IP_ADDR, rawstring);
	            break;

	            case 1:
	                strcpy(PORT, rawstring);
	                argv[0] = (char *)malloc(sizeof(char) * (strlen(rawstring)+1));
	                strcpy(argv[0] , PORT);
	            break;

	            case 2://FLAG WLAN
	                strcpy(WLAN, rawstring);
	                argv[4] = (char *)malloc(sizeof(char) * (strlen(rawstring)+1));
	                strcpy(argv[4] , WLAN);

	            break;

	            case 3://BOARD NAME
	                strcpy(PHY, rawstring);
	                argv[3] = (char *)malloc(sizeof(char) * (strlen(rawstring)+1));
	                strcpy(argv[3] , PHY);
	            break;

	            case 4://TIMESAMPLE MAC LAYER PARAMETERS
	                argv[2] = (char *)malloc(sizeof(char) * (strlen(rawstring)+1));
	                strcpy(argv[2], rawstring);
	            break;
	            case 5://PRINT FILE A/O SCREEN
	                argv[1] = (char *)malloc(sizeof(char) * (strlen(rawstring)+1));
	                strcpy(argv[1], rawstring);
	            break;
	            case 6:
	                strcpy(ESSID, rawstring);
	            break;

	            case 7://FOLDER
	                argv[5] = (char *)malloc(sizeof(char) * (strlen(rawstring)+1));
	                strcpy(argv[5], rawstring);
	            break;
	            case 8://subnet
	                strcpy(IP_SUBNET, rawstring);
	                argv[6] = (char *)malloc(sizeof(char) * (strlen(rawstring)+1));
	                strcpy(argv[6], rawstring);
	            break;
	            case 9://netmask
	                strcpy(IP_NETMASK, rawstring);
	                argv[7] = (char *)malloc(sizeof(char) * (strlen(rawstring)+1));
	                strcpy(argv[7], rawstring);
	            break;
	        }
	     }
//	    __android_log_print(ANDROID_LOG_DEBUG, "OPEN", "IPADDRESS %s" , IP_ADDR);
//	    __android_log_print(ANDROID_LOG_DEBUG, "OPEN", "IPADDRESS %d" , stringCount);
//	    __android_log_print(ANDROID_LOG_DEBUG, "OPEN", "SUBNET %s" , IP_SUBNET);
//	    __android_log_print(ANDROID_LOG_DEBUG, "OPEN", "NETMASK %s" , IP_NETMASK);

	    	    __android_log_print(ANDROID_LOG_DEBUG, "OPEN", "MATTEO");
    
//	    sprintf(scriptToLaunch, "su -c \"sh /sdcard/COGNET_TESTBED/SCRIPT/launcher.sh %s %s DURIP 0 %s %s\"" ,WLAN , IP_ADDR ,PHY, ESSID);
//	        	__android_log_print(ANDROID_LOG_DEBUG, "OPEN", "IPADDRESS %s" , scriptToLaunch);
	sprintf(scriptToLaunch, "su -c \"sh /system/bin/launcher.sh %s %s DURIP 0 %s %s\"" ,WLAN , IP_ADDR ,PHY, ESSID);
//	 _android_log_print(ANDROID_LOG_DEBUG, "OPEN", "%s " , scriptToLaunch);
	system(scriptToLaunch);
    if(ctrlFirst == 1){
    	mainReadMacServer(stringCount-1, argv);
    	ctrlFirst = 0;
    }else{
	    __android_log_print(ANDROID_LOG_DEBUG, "OPEN", "MATTEO222");
    }
	return 1;
}

JNIEXPORT jint JNICALL Java_it_durip_1app_ServerSocketCmd_startExperiment(JNIEnv *env, jobject thisObj ,
		jint n, jobjectArray stringArray){
	int ii;
    char filename[15];
    char portno[5];
    char sleepingTime[5];
    char **argv;
	int stringCount = (*env)->GetArrayLength(env, stringArray);
    argv = (char **) malloc(sizeof( char *) * stringCount);
	for(ii=0; ii < stringCount; ii++) {
		jstring element = (*env)->GetObjectArrayElement(env, stringArray, ii);
		const char * rawstring = (*env)->GetStringUTFChars(env, element, NULL);
        switch (ii) {
            case 0:
                strcpy(filename, rawstring);
            break;

            case 1:
                strcpy(portno, rawstring);

            break;

            case 2://FLAG WLAN
                strcpy(sleepingTime, rawstring);
            break;
        }
	 }
	return manageNetworkTestbed(portno , filename ,sleepingTime);
}

JNIEXPORT jint JNICALL Java_it_durip_1app_ServerSocketCmd_stopExperiment(JNIEnv *env, jobject thisObj ,
		jint n, jobjectArray stringArray){
	int ii;
    char filename[15];
    char  portno[5];

	int stringCount = (*env)->GetArrayLength(env, stringArray);

	for(ii=0; ii < stringCount; ii++) {
		jstring element = (*env)->GetObjectArrayElement(env, stringArray, ii);
		const char * rawstring = (*env)->GetStringUTFChars(env, element, NULL);
        switch (ii) {
            case 0:
                strcpy(filename, rawstring);
            break;
            case 1:
                strcpy(portno, rawstring);
            break;

        }
	 }
	return stopFunction(portno , filename);
}


JNIEXPORT jint JNICALL Java_it_durip_1app_ServerSocketCmd_killApp(JNIEnv *env, jobject thisObj ,
		jint n){

	return closeAPP();
}

