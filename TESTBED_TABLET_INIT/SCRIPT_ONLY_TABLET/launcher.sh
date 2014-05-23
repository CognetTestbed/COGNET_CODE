#!/system/bin/bash

insmod /system/lib/modules/mac80211.ko
insmod /system/lib/modules/ath.ko
insmod /system/lib/modules/ath9k_hw.ko
insmod /system/lib/modules/ath9k_common.ko
insmod /system/lib/modules/ath9k_htc.ko
iwconfig $1 mode ad-hoc essid $6  
iwconfig
echo "Setup ip"
ifconfig $1 $2
echo "INSERT DURIP MODULE"
insmod /sdcard/COGNET_TESTBED/KERNEL_MODULE/durip_nexus.ko checkPrint=$4
TCC=net.ipv4.tcp_congestion_control
sysctl $TCC
sysctl -w $TCC=$3
echo "CHANGE PERMISSION" 
chmod -R 777 /sys/kernel/debug/ieee80211/$5/netdev:$1/stations/
chmod -R 777 /sys/kernel/debug/ieee80211/$5/ath9k_htc/

