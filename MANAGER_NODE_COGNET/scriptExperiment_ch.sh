#!/bin/sh
iter=${1}
portstart=${2}
portstop=`expr $portstart + 1`
filename=${3}
filenameBand=${4}
sleeptime=${5}
secsleep=${6}

IP_iperf=${7}
time_iperf=${8}
interval_iperf=${9}
channel=${10}
nameDir=${11}
echo ${nameDir}
absolutePath="${HOME}/Dropbox/COGNET/BANDWIDTH/"
namePath=${absolutePath}${nameDir}
echo ${namePath}
mkdir ${namePath}


j="1"
while [ $j -lt $channel ]
do
i="0"
sleep $sleeptime
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
./manageNodeDurip.out startContinue $portstart $filename $sleeptime
sleep `expr $sleeptime + $sleeptime`
echo '###############START###############'
name=$filenameBand\_$m\_$d\_$hh\_$mm\_$ss\_CH\_$j
echo $name
iperf -c $IP_iperf -t $time_iperf -i $interval_iperf > $name
echo '###############STOP###############'
sleep $secsleep
./manageNodeDurip.out changeChannel $portstop $filename $j
mv $name $namePath
i=`expr $i + 1` 
sleep $secsleep
./manageNodeDurip.out stopContinue $portstop $filename
done
j=`expr $j + 1` 
done

