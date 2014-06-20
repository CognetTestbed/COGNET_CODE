#!/usr/bin/sh

#Cognetive Network programm 
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
#
case "$1" in
"nexus")
        echo "COMPILE COGNET TCP MODULE FOR NEXUS"
        make nexus
	mv cognetTCP.ko cognetTCP_nexus.ko
	#adb push cognetTCP_nexus.ko /system/lib/modules/
	adb push cognetTCP_nexus.ko /sdcard/COGNET_TESTBED/KERNEL_MODULE/
        ;;
"samsung")
        echo "COMPILE COGNET TCP MODULE FOR SAMSUNG"
        make samsung
	mv cognetTCP.ko cognetTCP_samsung.ko
	adb push cognetTCP_samsung.ko /sdcard/COGNET_TESTBED/KERNEL_MODULE/
        ;;
"pc")
	echo "COMPILE COGNET TCP MODULE FOR PC"
	make pc
	;;
"alix")
	echo "COMPILE COGNET TCP MODULE FOR ALIX"
	export ARCH=i386
	make alix
	mv cognetTCP.ko cognetTCP_alix.ko
	;;

"alix_new")
	echo "COMPILE COGNET TCP MODULE FOR ALIX"
	export ARCH=i386
	make alix_new
	mv cognetTCP.ko cognetTCP_alix.ko
;;
"compile")
	echo "ONLY COMPILE COGNET TCP FOR NEXUS"
	make nexus
	;;

*)
        echo "WRONG CHOICE. SELECT BETWEEN nexus or samsung"      
        ;;
esac
