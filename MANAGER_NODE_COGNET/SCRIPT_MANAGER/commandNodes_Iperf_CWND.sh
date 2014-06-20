#!/bin/sh
#number of interation port fileIP namefileBandwidth sleepingtime secSleep ipiperf  iperfduration intervaltime CWND NAME_FOLDER_BANDAWIDTH
./scriptExperiment_CWND.sh 10 4000 ../fileIP_localhost ${1} 5 10 192.168.1.${3} 30 1 $4 ${2}_CWND_${4}
