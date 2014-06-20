#!/bin/sh
i="0"
iter=$1
portstart=$2
portstop=`expr $portstart + 1`
filename=$3
filenameBand=$4
sleeptime=$5
secsleep=$6

IP_iperf=$7
time_iperf=$8
interval_iperf=$9
while [ $i -lt $iter ]
do
date=`date +'%Y-%m-%d %H:%M:%S'`
echo $date
Y=`date +'%Y'`
m=`date +'%m'`
d=`date +'%d'`
hh=`date +'%H'`
mm=`date +'%M'`
ss=`date +'%S'`
#echo $portstart 
#echo $filename
./manageNodeDurip.out startContinue $portstart $filename $sleeptime
sleep `expr $sleeptim + $sleeptime`
echo '###############START###############'
name=$filenameBand\_$m\_$d\_$hh\_$mm\_$ss
echo $name
iperf -c $IP_iperf -t $time_iperf -i $interval_iperf > $name
echo '###############STOP###############'
mv $name ~/Dropbox/COGNET/BANDWIDTH/
i=`expr $i + 1` 
sleep $secsleep
./manageNodeDurip.out stopContinue $portstop $filename
done


