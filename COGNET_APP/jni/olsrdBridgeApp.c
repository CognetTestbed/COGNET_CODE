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
#include "it_durip_app_Olsr.h"



JNIEXPORT jint JNICALL Java_it_durip_1app_Olsr_olsrdOn(JNIEnv *env, jobject thisObj ,
		jint n){
	char ss[128];
	sprintf(ss, "su -c \"nohup /system/bin/olsrd -f /sdcard/COGNET_TESTBED/CONFIG/olsrdTABLET.conf > /dev/null &\"");


	system(ss);
	return 1;
}





JNIEXPORT jint JNICALL Java_it_durip_1app_Olsr_olsrdOff(JNIEnv *env, jobject thisObj ,
		jint n){
	char ss[100];
	//-INT
	sprintf(ss, "su -c \"kill -INT %d\"", n);

	__android_log_print(ANDROID_LOG_DEBUG, "OPEN", "OLSRD OFF: %s" , ss);
	//check if modules are up or not
	system(ss);
	return 1;
}






