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

	return 0;
//	return closeAPP();
}

