#include <jni.h>
#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "it_durip_app_ServerSocketCmd.h"
#include "serverThread.h"



JNIEXPORT jint JNICALL Java_it_durip_1app_ServerSocketCmd_mainJNI(JNIEnv *env, jobject thisObj ,
		jint n, jobjectArray stringArray){
	
	int ii;
	char scriptToLaunch[128];
	char CognetToLaunch[128];
	char IP_ADDR[15];
	char IP_SUBNET[15];
	char IP_NETMASK[15];
	char WLAN[8];
	char PHY[8];
	char PORT[8];
	char ESSID[32];
	char TS[8];
	char TYPEOUTPUT[4];
	char FOLDER[16];
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
				strcpy(TS,rawstring);
			break;
			case 5://PRINT FILE A/O SCREEN
				argv[1] = (char *)malloc(sizeof(char) * (strlen(rawstring)+1));
				strcpy(argv[1], rawstring);
				strcpy(TYPEOUTPUT , rawstring);
			break;
			case 6:
				strcpy(ESSID, rawstring);
			break;

			case 7://FOLDER
				argv[5] = (char *)malloc(sizeof(char) * (strlen(rawstring)+1));
				strcpy(argv[5], rawstring);
				strcpy(FOLDER,  rawstring);
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


	sprintf(scriptToLaunch, "su -c \"sh /sdcard/COGNET_TESTBED/SCRIPT/launcher.sh %s %s DURIP 0 %s %s\"" ,WLAN , IP_ADDR ,PHY, ESSID);
	        	__android_log_print(ANDROID_LOG_DEBUG, "OPEN", "IPADDRESS %s" , scriptToLaunch);

	system(scriptToLaunch);

#if ANDROID_EXE == 0
	mainReadMacServer(stringCount-2, argv);
#else

	__android_log_print(ANDROID_LOG_DEBUG, "OPEN", "RUN COGNET_CLIENT %s" , scriptToLaunch);
	sprintf(CognetToLaunch, "su -c \"/system/bin/COGNET_CLIENT %s %s %s %s %s %s %s %s & \"" ,PORT , TYPEOUTPUT, TS ,PHY ,WLAN , FOLDER, IP_SUBNET , IP_NETMASK );
	system(CognetToLaunch);
#endif


	return 1;
}
