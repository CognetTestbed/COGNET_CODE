#!/usr/bin/sh
adb root
sleep 5
adb shell mount -o remount,rw -t yaffs2 /dev/block/mtdblock4 /system
adb shell mkdir -p /sdcard/COGNET_TESTBED/
adb shell mkdir -p /sdcard/COGNET_TESTBED/PLUGIN_OLSRD/
adb shell mkdir -p /sdcard/COGNET_TESTBED/KERNEL_MODULE/
adb shell mkdir -p /sdcard/COGNET_TESTBED/SCRIPT/
adb shell mkdir -p /sdcard/COGNET_TESTBED/BIN/
adb shell mkdir -p /sdcard/COGNET_TESTBED/CONFIG/
adb shell mkdir -p /sdcard/local/log/

adb push 86mountSystem /etc/init.d/
adb push iwconfig /system/bin/
adb push iperf /system/bin/
adb push tcpOutput.sh /sdcard/COGNET_TESTBED/SCRIPT/

adb push htc_9271.fw /etc/firmware/
adb shell chmod 755 /system/bin/iwconfig
adb shell chmod 755 /system/bin/iperf
adb shell chmod 755 /etc/init.d/86mountSystem
adb shell chmod 755 /sdcard/COGNET_TESTBED/SCRIPT/tcpOutput.sh

adb push launcher.sh /sdcard/COGNET_TESTBED/SCRIPT/
#adb push launcher.sh /system/bin/
#adb shell chmod 755 /system/bin/launcher.sh
adb shell chmod 755 /sdcard/COGNET_TESTBED/SCRIPT/launcher.sh

adb push scriptOLSRD_OUTPUT.sh /sdcard/COGNET_TESTBED/SCRIPT/
adb shell chmod 755 /sdcard/COGNET_TESTBED/SCRIPT/scriptOLSRD_OUTPUT.sh


adb push cognetTCP_nexus.ko /sdcard/COGNET_TESTBED/KERNEL_MODULE/
#cd ../../COGNET_TCP_MODULE/
#sh $(pwd)/compileModule.sh nexus
#cd -
##ADD OLSRD, PLUGIN and CONFIG
cd ../../MULTIHOP_ROUTE_SW/olsrd_telnet/
sh $(pwd)/MAKE_ANDROID.sh
cd -
adb push olsrdTABLET.conf /sdcard/COGNET_TESTBED/CONFIG/
adb shell chmod 755 /system/bin/olsrd

adb shell mkdir -p /sdcard/local/OLSRD_TEST/
