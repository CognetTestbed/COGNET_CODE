CC=gcc




CFLAGS=-Os -W -Wall -I .


all: 
	gcc -o manageNodeDurip.out manageNodeDurip.c -lm   $(CFLAGS) 

client:
	gcc -o clientSYNC.out clientSYNC.c  -lm $(CFLAGS) 


.PHONY: clean

clean:
	rm -f ./*.out *~ 
