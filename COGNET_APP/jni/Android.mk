
LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := olsrdLib
LOCAL_SRC_FILES:=olsrdBridgeApp.c 
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := olsrdLogLib
LOCAL_SRC_FILES:=bridgeOlsrdLog.c
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := hostsLib
LOCAL_SRC_FILES:=hostsBridgeApp.c 
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)


MY_LOCAL_PATH_NL:=../../LIB_NETLINK/libnl-3-android-nogit
$(warning $(MY_LOCAL_PATH_NL))
include $(CLEAR_VARS)
LOCAL_MODULE := nl-3
LOCAL_SRC_FILES := $(MY_LOCAL_PATH_NL)/android_toolchain/libs/$(TARGET_ARCH_ABI)/libnl-3.so
LOCAL_C_INCLUDES:=$(MY_LOCAL_PATH_NL)/include/\
		  $(MY_LOCAL_PATH_NL)/android_toolchain/jni/missing_include/\
		  $(MY_LOCAL_PATH_NL)/android_toolchain/jni/generated_include
include $(PREBUILT_SHARED_LIBRARY)



include $(CLEAR_VARS)
LOCAL_MODULE := nl-genl-3
LOCAL_SRC_FILES := $(MY_LOCAL_PATH_NL)/android_toolchain/libs/$(TARGET_ARCH_ABI)/libnl-genl-3.so
LOCAL_C_INCLUDES:=$(MY_LOCAL_PATH_NL)/include/ \
		  $(MY_LOCAL_PATH_NL)/android_toolchain/jni/missing_include \
		  $(MY_LOCAL_PATH_NL)/android_toolchain/jni/generated_include
include $(PREBUILT_SHARED_LIBRARY)



include $(CLEAR_VARS)
MY_LOCAL_SRC_DIR:=../../COGNET_CLIENT_PARAMETERS
MY_LOCAL_SRC_DIR_MANAGE:=../../MANAGER_NODE_COGNET_TABLET
LOCAL_CFLAGS := -DATH9K_HTC=1  
#LOCAL_CFLAGS := -DTEST_MATTEO=1 -g
LOCAL_CFLAGS += -DANDROID_EXE=1
LOCAL_MODULE    := MACReadServer
LOCAL_SRC_FILES:= bridgeApp_MAC.c
#LOCAL_SRC_FILES+=$(MY_LOCAL_SRC_DIR_MANAGE)/managerNodeCognet.c

##LOCAL_SRC_FILES+=$(MY_LOCAL_SRC_DIR)/serverCOGNET.c\
#				 $(MY_LOCAL_SRC_DIR)/libTHREADS_MAC/macObservation.c\
#				 $(MY_LOCAL_SRC_DIR)/libTHREADS_MAC/printMACvalue.c \
#				 $(MY_LOCAL_SRC_DIR)/libTHREADS_MAC/commonFunction.c \
#				 $(MY_LOCAL_SRC_DIR)/libTHREADS_MAC/CognetThreadControlRead.c \
#				 $(MY_LOCAL_SRC_DIR)/libTHREADS_MAC/macChangeParams.c \
#				 $(MY_LOCAL_SRC_DIR)/libTHREADS_MAC/tcpObservation.c \
#				 $(MY_LOCAL_SRC_DIR)/libTHREADS_MAC/macThreadComm.c \
#				 $(MY_LOCAL_SRC_DIR)/libTHREADS_MAC/get_station.c \
#				  
#				 
LOCAL_C_INCLUDES:=$(MY_LOCAL_SRC_DIR)/include
#LOCAL_C_INCLUDES+=$(MY_LOCAL_PATH_NL)/include/ \
#		  $(MY_LOCAL_PATH_NL)/android_toolchain/jni/missing_include \
#		  $(MY_LOCAL_PATH_NL)/android_toolchain/jni/generated_include 


LOCAL_SHARED_LIBRARIES:=nl-3 nl-genl-3				 
				 
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
MY_LOCAL_SRC_DIR:=../../COGNET_CLIENT_PARAMETERS
MY_LOCAL_SRC_DIR_MANAGE:=../../MANAGER_NODE_COGNET_TABLET

LOCAL_MODULE    := ExperimentManager
LOCAL_SRC_FILES:=experimentManager.c
LOCAL_SRC_FILES+=$(MY_LOCAL_SRC_DIR_MANAGE)/managerNodeCognet.c

				 
LOCAL_C_INCLUDES:=$(MY_LOCAL_SRC_DIR)/include

				 
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_EXECUTABLE)
