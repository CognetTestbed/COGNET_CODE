#!/usr/bin/sh

while true; do 
wget http://127.0.0.1:8081/all -qO - >> /sdcard/local/OLSRD_TEST/$1
echo "####" >> /sdcard/local/OLSRD_TEST/$1
sleep $2; 
done
