#include <jni.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>


#include <android/log.h>
#include "it_durip_app_olsrdLog.h"


JNIEXPORT jint JNICALL Java_it_durip_1app_olsrdLog_olsrdLogOn(JNIEnv *env, jobject thisObj ,
		jint n, jobjectArray stringArray){

		int ii;
	    char filename[32];
	    char sampletime[8];
	    char ss[128];
	    int stringCount = (*env)->GetArrayLength(env, stringArray);


	    for(ii=0; ii < stringCount; ii++) {
	        jstring element = (*env)->GetObjectArrayElement(env, stringArray, ii);
	        const char * rawstring = (*env)->GetStringUTFChars(env, element, NULL);
	        switch (ii) {
	            case 0:

	                strcpy(filename, rawstring);
	                __android_log_print(ANDROID_LOG_DEBUG, "OLSRD LOG",filename);
	            break;

	            case 1:
	                strcpy(sampletime, rawstring);
	                __android_log_print(ANDROID_LOG_DEBUG, "OLSRD LOG",sampletime);
	            break;
	        }
	     }

//	    sprintf(ss, "su -c \"nohup sh /sdcard/COGNET_TESTBED/SCRIPT/scriptOLSRD_OUTPUT.sh %s %s & echo $! > PID.txt  \"", filename , sampletime );
//	    sprintf(ss, "sh /sdcard/COGNET_TESTBED/SCRIPT/scriptOLSRD_OUTPUT.sh %s %s & echo $! > PID.txt  ", filename , sampletime );
	    
	    sprintf(ss, "sh /sdcard/COGNET_TESTBED/SCRIPT/scriptOLSRD_OUTPUT.sh %s %d &", filename ,  1000 * atoi(sampletime) );
	    __android_log_print(ANDROID_LOG_DEBUG, "OLSRD LOG",ss);
	    system(ss);
//	    system("echo $! > /sdcard/COGNET_TESTBED/SCRIPT/PID.txt");
	    return 1;
}


JNIEXPORT jint JNICALL Java_it_durip_1app_olsrdLog_olsrdLogOff(JNIEnv *env, jobject thisObj ,
		jint n){
	char ss[100];
	//-INT
	sprintf(ss, "su -c \"kill -INT `ps | grep sh | awk '($1==\"u0_a49\" && $9==\"sh\") {print $2}'`\"");

	__android_log_print(ANDROID_LOG_DEBUG, "OPEN", "OLSRD OFF: %s" , ss);
	//check if modules are up or not
	system(ss);
	return 1;
}
