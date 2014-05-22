#!/usr/bin/sh


while true; do 
wget http://127.0.0.1:8081/all -qO - >> /mnt/local/log/OLSRD/$1
echo "####" >> /mnt/local/log/OLSRD/$1
sleep $2; 
done
