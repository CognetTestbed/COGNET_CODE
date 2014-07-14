#!/bin/bash
while true; do 
wget http://127.0.0.1:9090/links/routes -qO - >> $1/EXP_OUTPUT
echo "####" >> $1/EXP_OUTPUT
sleep $2; 
done
