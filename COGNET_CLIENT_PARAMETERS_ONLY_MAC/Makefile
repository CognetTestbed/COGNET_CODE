CC=gcc

PKG_CONFIG ?= pkg-config
#CFLAGS=-Os -W -Wall -I .

CFLAGS ?= -O2 -g
CFLAGS += -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration



pc:
	gcc -o COGNET_PC.out serverCOGNET.c ./libTHREADS_MAC/macObservation.c \
	    ./libTHREADS_MAC/printMACvalue.c ./libTHREADS_MAC/commonFunction.c \
	    ./libTHREADS_MAC/CognetThreadControlRead.c ./libTHREADS_MAC/macChangeParams.c \
	    ./libTHREADS_MAC/macThreadComm.c ./libTHREADS_MAC/routingTable.c ./libTHREADS_MAC/get_station.c -lrt -lm -lpthread $(LIBS)  $(CFLAGS) -DATH9K_HTC=0 -DATH5K=0\
		-L ../LIB_NETLINK/libnl-3/OUTPUT_NL_3_64bit/lib -lnl-genl-3 -lnl-3 -I../LIB_NETLINK/libnl-3/OUTPUT_NL_3_64bit/include/libnl3
		


alix:
	gcc -m32 -o COGNET_ALIX.out serverCOGNET.c ./libTHREADS_MAC/macObservation.c \
	    ./libTHREADS_MAC/printMACvalue.c ./libTHREADS_MAC/commonFunction.c \
	    ./libTHREADS_MAC/CognetThreadControlRead.c ./libTHREADS_MAC/macChangeParams.c\
	    ./libTHREADS_MAC/macThreadComm.c ./libTHREADS_MAC/get_station.c -lrt -lm -lpthread $(LIBS)  $(CFLAGS) -DATH9K_HTC=0 -DATH5K=0\
	-L ../LIB_NETLINK/libnl-3/OUTPUT_NL_3_32bit/lib -lnl-genl-3 -lnl-3 -I../LIB_NETLINK/libnl-3/OUTPUT_NL_3_32bit/include/libnl3

alix5k:
	gcc -m32 -o COGNET_ALIX.out serverCOGNET.c ./libTHREADS_MAC/macObservation.c \
	    ./libTHREADS_MAC/printMACvalue.c ./libTHREADS_MAC/commonFunction.c \
	    ./libTHREADS_MAC/CognetThreadControlRead.c ./libTHREADS_MAC/macChangeParams.c\
	    /libTHREADS_MAC/macThreadComm.c ./libTHREADS_MAC/get_station.c -lrt -lm -lpthread $(LIBS)  $(CFLAGS) -DATH9K_HTC=0 -DATH5K=1\
	-L ../LIB_NETLINK/libnl-3/OUTPUT_NL_3_32bit/lib -lnl-genl-3 -lnl-3 -I../LIB_NETLINK/libnl-3/OUTPUT_NL_3_32bit/include/libnl3

pcDoongle:
	gcc -o COGNET_PC.out serverCOGNET.c ./libTHREADS_MAC/macObservation.c \
	    ./libTHREADS_MAC/printMACvalue.c ./libTHREADS_MAC/commonFunction.c \
	    ./libTHREADS_MAC/CognetThreadControlRead.c ./libTHREADS_MAC/macChangeParams.c\
	    ./libTHREADS_MAC/macThreadComm.c ./libTHREADS_MAC/get_station.c -lrt -lm -lpthread $(LIBS)  $(CFLAGS) -DATH9K_HTC=1 -DATH5K=0\
	-L ../LIB_NETLINK/libnl-3/OUTPUT_NL_3_64bit/lib -lnl-genl-3 -lnl-3 -I../LIB_NETLINK/libnl-3/OUTPUT_NL_3_64bit/include/libnl3



alixDoongle:
	gcc -m32 -o COGNET_ALIX_doongle.out serverCOGNET.c ./libTHREADS_MAC/macObservation.c \
	    ./libTHREADS_MAC/printMACvalue.c ./libTHREADS_MAC/commonFunction.c \
	    ./libTHREADS_MAC/CognetThreadControlRead.c ./libTHREADS_MAC/macChangeParams.c\
	    ./libTHREADS_MAC/macThreadComm.c ./libTHREADS_MAC/get_station.c -lrt -lm -lpthread $(LIBS)  $(CFLAGS) -DATH9K_HTC=1 -DATH5K=0\
	-L ../LIB_NETLINK/libnl-3/OUTPUT_NL_3_32bit/lib -lnl-genl-3 -lnl-3 -I../LIB_NETLINK/libnl-3/OUTPUT_NL_3_32bit/include/libnl3 
	
	



.PHONY: clean

clean:
	rm -f ./*.out *~ 
