CC=gcc




CFLAGS=-Os -W -Wall -I .


all: 
	gcc -o managerNodeDurip.out manageNodeDurip.c -lm   $(CFLAGS) 

alix: 
	gcc -m32 -o managerCognetAlix.out manageNodeDurip.c -lm   $(CFLAGS) 

client:
	gcc -o clientSYNC.out clientSYNC.c  -lm $(CFLAGS) 


.PHONY: clean

clean:
	rm -f ./*.out *~ 
