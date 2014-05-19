#!/bin/bash


case "$1" in
"1")

adb root
make ./drivers/net/wireless/bcmdhd/bcmdhd.ko ARCH=arm CROSS_COMPILE=/locale/ANDROID/NEXUS7_NEWWIFI/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi- cyanogenmod_grouper_defconfig
sleep '1'
adb push drivers/net/wireless/bcmdhd/bcmdhd.ko /system/lib/modules/
adb reboot
;;
"2")
make ./drivers/net/wireless/bcmdhd/bcmdhd.ko ARCH=arm CROSS_COMPILE=/locale/ANDROID/NEXUS7_NEWWIFI/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-
;;

"3")
make ./drivers/net/wireless/ath/ath9k/ath9k_htc.ko cyanogenmod_grouper_defconfig ARCH=arm CROSS_COMPILE=/locale/ANDROID/NEXUS7_NEWWIFI/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi- 
#make ./net/mac80211/mac80211.ko cyanogenmod_grouper_defconfig ARCH=arm CROSS_COMPILE=/locale/ANDROID/NEXUS7_NEWWIFI/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi- 
#make ./net/wireless/cfg80211.ko cyanogenmod_grouper_defconfig ARCH=arm CROSS_COMPILE=/locale/ANDROID/NEXUS7_NEWWIFI/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi- 
;;
"4")
yes "" | make oldconfig ARCH=arm CROSS_COMPILE=/locale/ANDROID/NEXUS7_NEWWIFI/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-
make menuconfig ARCH=arm CROSS_COMPILE=/locale/ANDROID/NEXUS7_NEWWIFI/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi- 
;;
"5")
make modules ARCH=arm CROSS_COMPILE=/locale/ANDROID/NEXUS7_NEWWIFI/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-
;;
"6")
make menuconfig ARCH=arm CROSS_COMPILE=/locale/ANDROID/NEXUS7_NEWWIFI/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi- 
;;
esac
