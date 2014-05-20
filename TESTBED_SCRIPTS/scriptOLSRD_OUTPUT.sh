#!/usr/bin/sh


while true; do 
wget http://127.0.0.1:8081/all -qO - >> $1
echo "####" >> fileOutput 
sleep $2; 
done