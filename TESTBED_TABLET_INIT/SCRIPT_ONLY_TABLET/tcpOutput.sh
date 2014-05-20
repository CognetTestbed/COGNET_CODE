#!/usr/bin/bash
A="1"

while [ $A -lt "2" ] ; do
	cat /proc/tcp_output_durip
	sleep $1
done
