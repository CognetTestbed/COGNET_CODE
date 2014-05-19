#include <jni.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include<errno.h>
#include <time.h>
#include <regex.h>
//#include <libexplain/popen.h>

//#include "utlist.h"


#include <android/log.h>
#include "it_durip_app_DeviceSwitch.h"

JNIEXPORT jobjectArray JNICALL Java_it_durip_1app_DeviceSwitch_activeHosts(JNIEnv *env, jobject thisObj ,
		jint n){
	char ss[256];
	int i = 0;
	char line[1024];
	jstring tmp = NULL;

	sprintf(ss, "su -c \"head -%d /sdcard/hosts \"",(int)n);

	__android_log_print(ANDROID_LOG_DEBUG, "OPEN", "HOSTS ACTIVE: %s" ,ss);
	//check if modules are up or not

	jclass sclass = (*env)->FindClass(env, "java/lang/String");
	jobjectArray ret = (*env)->NewObjectArray(env, (int)n, sclass, NULL);

	FILE *fp;
	// Open the command for reading.
	//fp = explain_popen_or_die(ss, "r");
	fp = popen(ss, "r");
	if (!fp) {
		printf("Failed to run command\n" );
		exit;
	}
	// Read the output a line at a time - output it.
	while (fgets(line, sizeof(line)-1, fp) != NULL) {
		printf("%s", line);
		tmp = (*env)->NewStringUTF(env,line);
		(*env)->SetObjectArrayElement(env,ret,i++,tmp);
		(*env)->DeleteLocalRef(env,tmp);
	}
	pclose(fp);

	return ret;
}

JNIEXPORT jobjectArray JNICALL Java_it_durip_1app_DeviceSwitch_unactiveHosts(JNIEnv *env, jobject thisObj ,
		jint n){
    //char total[4][2] = {{"192.168.1.1","a0:f3:c1:1c:93:ea"},{"192.168.1.3","F8:1A:67:0C:78:DE"},{"192.168.1.120","a0:f3:c1:ed:de:82"},{"192.168.1.160","00:1d:7d:49:2b:e7"}};

	char ss[100];
	int i = 0;
	char line[1000];
	jstring tmp = NULL;
	sprintf(ss, "su -c \"iptables -L INPUT\"");

	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", "HOSTS UNACTIVE: %s" ,ss);
	//check if modules are up or not

	jclass sclass = (*env)->FindClass(env, "java/lang/String");
	jobjectArray ret = (*env)->NewObjectArray(env, (int)n, sclass, NULL);

	FILE *fp;

	//fp = popen(ss, "r");
	fp = popen(ss, "r");
	if (!fp) {
		printf("Failed to run command\n" );
		exit;
	}


	//we need capturing group 1
	//REJECT.*MAC\s(([[:xdigit:]]{2}[-: ]){5}[[:xdigit:]]{2}).*
	//more strict
	//REJECT[\s\w-\.]*MAC\s(([[:xdigit:]]{2}[-: ]){5}[[:xdigit:]]{2})
	regex_t compiled;
	regcomp(&compiled, "REJECT.*MAC\\s(([[:xdigit:]]{2}[-: ]){5}[[:xdigit:]]{2}).*", REG_EXTENDED);
	size_t ngroups = compiled.re_nsub + 1;
	regmatch_t *groups = malloc(ngroups * sizeof(regmatch_t));
	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", "Hcompiled regex:");
	while (fgets(line, sizeof(line)-1, fp) != NULL) {
		regexec(&compiled, line, ngroups, groups, 0);
		__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", "read line: %s",line);
		tmp = (*env)->NewStringUTF(env,line);
		(*env)->SetObjectArrayElement(env,ret,i++,tmp);
		(*env)->DeleteLocalRef(env,tmp);
	}
	pclose(fp);

	return ret;
}

JNIEXPORT jint JNICALL Java_it_durip_1app_DeviceSwitch_hostOn(JNIEnv *env, jobject thisObj ,
		jstring macUndrop){
	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", " ON first row");
	const char *ss = (*env)->GetStringUTFChars(env,macUndrop,NULL);
	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", "macaddress: %s" , ss);
	char sx[80];
	sprintf(sx, "su -c \"iptables -D INPUT -m mac --mac-source %s -j REJECT\"", ss);

	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", "launch: %s", sx);
	system(sx);
	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", "done!!");
	return 1;
}

JNIEXPORT jint JNICALL Java_it_durip_1app_DeviceSwitch_hostOff(JNIEnv *env, jobject thisObj ,
		jstring macDrop){
	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", " OFF first row");
	const char *ss = (*env)->GetStringUTFChars(env,macDrop,NULL);
	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", "macaddress: %s" , ss);
	char sx[80];
	sprintf(sx, "su -c \"iptables -A INPUT -m mac --mac-source %s -j REJECT\"", ss);

	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", "launch: %s", sx);
	system(sx);
	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", "done!!");
	return 1;
}

JNIEXPORT jint JNICALL Java_it_durip_1app_DeviceSwitch_flush(JNIEnv *env, jobject thisObj){
	char sx[80];
	sprintf(sx, "su -c \"iptables -F INPUT\"");
	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", "launch: %s", sx);
	system(sx);
	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", "done!!");
	return 1;
}

JNIEXPORT jint JNICALL Java_it_durip_1app_DeviceSwitch_fill(JNIEnv *env, jobject thisObj){
	char sx[80];
	sprintf(sx, "su -c \"sh /sdcard/scriptIPTABLES.sh\"");
	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", "launch: %s", sx);
	system(sx);
	__android_log_print(ANDROID_LOG_DEBUG, "DEVICESWITCH", "done!!");
	return 1;
}
