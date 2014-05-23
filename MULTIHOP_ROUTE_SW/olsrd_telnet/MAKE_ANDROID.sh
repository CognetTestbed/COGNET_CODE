#!/usr/sh
make clean
make OS=android NDK_BASE=/locale/ANDROID/android-ndk-r8e build_all

#INSTALLATION PART
adb root
adb push olsrd /system/bin/
adb push ./lib/httpinfo/olsrd_httpinfo.so.0.1 /sdcard/COGNET_TESTBED/PLUGIN_OLSRD/
adb push ./lib/txtinfo/olsrd_txtinfo.so.0.1 /sdcard/COGNET_TESTBED/PLUGIN_OLSRD/
adb push ./lib/telnet/olsrd_telnet.so.0.1 /sdcard/COGNET_TESTBED/PLUGIN_OLSRD/
