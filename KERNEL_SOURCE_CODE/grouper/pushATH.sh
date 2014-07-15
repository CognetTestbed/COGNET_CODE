#!/usr/bin/sh
PATH_ATH=./drivers/net/wireless/ath
PATH_MAC=./net/mac80211/
	adb push $PATH_ATH/ath.ko /system/lib/modules/
	adb push $PATH_ATH/ath9k/ath9k.ko /system/lib/modules/
	adb push $PATH_ATH/ath9k/ath9k_common.ko /system/lib/modules/
	adb push $PATH_ATH/ath9k/ath9k_htc.ko /system/lib/modules/
	adb push $PATH_ATH/ath9k/ath9k_hw.ko /system/lib/modules/
	adb push $PATH_MAC/mac80211.ko /system/lib/modules/
	adb push ./net/wireless/cfg80211.ko /system/lib/modules/
