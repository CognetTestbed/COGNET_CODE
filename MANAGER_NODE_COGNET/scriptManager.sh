#!/bin/sh
i="0"
iter=$1
portstart=$2
portstop=`expr $portstart + 1`
filename=$3
sleeptime=$4
sec=$5
while [ $i -lt $iter ]
do
echo '###############START###############'
echo $i $portstart $portstop $filename $sec
manageNodeDurip.out startContinue $portstart $filename $sleeptime
sleep $sec
i=`expr $i + 1`
echo '###############STOP###############'
manageNodeDurip.out stopContinue $portstop $filename
done
