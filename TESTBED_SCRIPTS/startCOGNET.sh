#!/bin/bash
DIR_OUTPUT=ONLY_START
PORT=4000
TS=100

OLSR_TS=1

TYPE_OUTPUT=2
#ntpdate 147.162.97.20 
Y=`date +'%Y'`
m=`date +'%m'`
d=`date +'%d'`
hh=`date +'%H'`
mm=`date +'%M'`
ss=`date +'%S'`

P=`pgrep olsrd`
#echo $P
if [ `pgrep olsrd` != $'\n' ]; 
then
	OUTPUT_OLSR=/mnt/local/log/OLSRD/EXP_YY${Y}_m${m}_d${d}_h${hh}_mm${mm}
	mkdir ${OUTPUT_OLSR}
	./scriptOLSRD_OUTPUT.sh ${OUTPUT_OLSR} ${OLSR_TS} &
else
	echo "NO OLSRD" 
fi


#./COGNET_ALIX.out ${PORT} ${TYPE_OUTPUT} ${TS} phy0 wlan0 ${DIR_OUTPUT} 192.168.1.0 255.255.255.0
