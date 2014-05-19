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
name=$filenameBand\_$m\_$d\_$hh\_$mm\_$ss\_CH\_$j
echo $name
iperf -c $IP_iperf -t $time_iperf -i $interval_iperf > $name
echo '###############STOP###############'
sleep $secsleep
mv $name $namePath
sleep $secsleep
../manageNodeDurip.out stopContinue $portstop $filename

