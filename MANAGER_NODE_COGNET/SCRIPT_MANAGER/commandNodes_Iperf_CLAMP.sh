#!/bin/sh
#number of interation port fileIP namefileBandwidth sleepingtime secSleep ipiperf  iperfduration intervaltime CLAMP NAME_FOLDER_BANDAWIDTH
./scriptExperiment_CLAMP.sh 3 4000 ../fileIP_localhost ${1} 5 10 192.168.1.${3} 30 1 $4 ${2}_CLAMP_${4}
