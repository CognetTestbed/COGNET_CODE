#include <jni.h>
#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "it_durip_app_ExperimentMasterActivity.h"
#include "serverThread.h"
//#include "jniMainActivity.h"

JNIEXPORT jint JNICALL Java_it_durip_1app_ExperimentMasterActivity_mainJNI(JNIEnv *env, jobject thisObj ,
		jint n, jobjectArray stringArray){

	int ii;
    char ss[100];
    char IP_ADDR[15];
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
        }
	 }
    __android_log_print(ANDROID_LOG_DEBUG, "OPEN", "IPADDRESS %s" , IP_ADDR);
    sprintf(ss, "su -c \"sh /system/bin/launcher.sh %s %s DURIP 0 %s %s\"" ,WLAN , IP_ADDR ,PHY, ESSID);
    
    __android_log_print(ANDROID_LOG_DEBUG, "OPEN", "MAIN %s" , ss);
    system(ss);
    mainReadMacServer(stringCount-1, argv);
	return 1;
}

JNIEXPORT jint JNICALL Java_it_durip_1app_ExperimentMasterActivity_startExperiment(JNIEnv *env, jobject thisObj ,
		jint n, jobjectArray stringArray){

	__android_log_print(ANDROID_LOG_DEBUG, "EXPERIMENT", "start!!");
	int ii;
    char filename[15];
    char portno[5];
    char sleepingTime[5];

    char **argv;
	int stringCount = (*env)->GetArrayLength(env, stringArray);
//	__android_log_print(ANDROID_LOG_DEBUG, "EXPERIMENT", "getarr");
    argv = (char **) malloc(sizeof( char *) * stringCount);

	for(ii=0; ii < stringCount; ii++) {
		jstring element = (*env)->GetObjectArrayElement(env, stringArray, ii);
		const char * rawstring = (*env)->GetStringUTFChars(env, element, NULL);
//		__android_log_print(ANDROID_LOG_DEBUG, "EXPERIMENT", rawstring);
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
	__android_log_print(ANDROID_LOG_DEBUG, "EXPERIMENT", "return");
	return manageNetworkTestbed(portno , filename ,sleepingTime);
}

JNIEXPORT jint JNICALL Java_it_durip_1app_ExperimentMasterActivity_stopExperiment(JNIEnv *env, jobject thisObj ,
		jint n, jobjectArray stringArray){

	__android_log_print(ANDROID_LOG_DEBUG, "EXPERIMENT", "stop begins");
	int ii;
    char filename[15];
    char  portno[5];

    char **argv;
	int stringCount = (*env)->GetArrayLength(env, stringArray);

    argv = (char **) malloc(sizeof( char *) * stringCount);

	for(ii=0; ii < stringCount; ii++) {
		jstring element = (*env)->GetObjectArrayElement(env, stringArray, ii);
		const char * rawstring = (*env)->GetStringUTFChars(env, element, NULL);
        switch (ii) {
            case 0:
                strcpy(portno, rawstring);
            break;
            case 1:
            	strcpy(filename, rawstring);
            break;

        }
	 }
	__android_log_print(ANDROID_LOG_DEBUG, "EXPERIMENT", "stop");
	return stopFunction(portno , filename);
}


JNIEXPORT jint JNICALL Java_it_durip_1app_ExperimentMasterActivity_killApp(JNIEnv *env, jobject thisObj ,
		jint n){

	return closeAPP();
}

