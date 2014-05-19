#Cognitive Network programm 
#Copyright (C) 2014  Matteo Danieletto matteo.danieletto@dei.unipd.it
#University of Padova, Italy +34 049 827 7778
#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.


LOCAL_PATH:=$(call my-dir)
	
MY_LOCAL_PATH_NL:=../libnl-3-android-nogit
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

LOCAL_SRC_FILES:=serverCognet.c 
LOCAL_SRC_FILES+=macObservation.c libTHREADS_MAC/printMACvalue.c libTHREADS_MAC/commonFunction.c \
		libTHREADS_MAC/CognetThreadControlRead.c libTHREADS_MAC/macChangeParams.c\
		 libTHREADS_MAC/tcpObservation.c libTHREADS_MAC/macThreadComm.c libTHREADS_MAC/get_station.c

LOCAL_C_INCLUDES:=$(MY_LOCAL_PATH_NL)/include/ \
		  $(MY_LOCAL_PATH_NL)/android_toolchain/jni/missing_include \
		  $(MY_LOCAL_PATH_NL)/android_toolchain/jni/generated_include


#$(warning $(LOCAL_C_INCLUDES))
LOCAL_SHARED_LIBRARIES:=nl-3 nl-genl-3

LOCAL_MODULE:=DURIP_READ_SERVER
LOCAL_LDLIBS := -llog 
include $(BUILD_EXECUTABLE)
