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
CLAMP=${10}
nameDir=${11}
echo ${nameDir}
absolutePath="${HOME}/Dropbox/COGNET/BANDWIDTH/"
namePath=${absolutePath}${nameDir}
echo ${namePath}
mkdir ${namePath}


i="0"
sleep $sleeptime
while [ $i -lt $iter ]
do
echo ${CLAMP} > /proc/tcp_input_cwnd
date=`date +'%Y-%m-%d %H:%M:%S'`
echo $date
Y=`date +'%Y'`
m=`date +'%m'`
d=`date +'%d'`
hh=`date +'%H'`
mm=`date +'%M'`
ss=`date +'%S'`
../manageNodeDurip.out startContinue $portstart $filename $sleeptime
sleep `expr $sleeptime + $sleeptime`
echo '###############START###############'
name=$filenameBand\_$m\_$d\_$hh\_$mm\_$ss\_CLAMP
echo $name
iperf -c $IP_iperf -t $time_iperf -i $interval_iperf > $name
echo '###############STOP###############'
sleep $secsleep
mv $name $namePath
i=`expr $i + 1` 
sleep $secsleep
../manageNodeDurip.out stopContinue $portstop $filename
done
