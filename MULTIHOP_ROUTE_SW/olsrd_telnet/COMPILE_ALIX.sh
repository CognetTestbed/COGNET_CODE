#!/usr/bin/sh

make -f MakefileAlix
cd ./lib/httpinfo/

make -f MakefileAlix 
cd -
cd ./lib/telnet/
make -f MakefileAlix 

cd -
scp olsrdAlix root@10.1.129.131:/root/COGNET_TESTBED/
scp olsrdAlix root@10.1.129.133:/root/COGNET_TESTBED/
scp olsrdAlix root@10.1.129.134:/root/COGNET_TESTBED/
scp olsrdAlix root@10.1.129.135:/root/COGNET_TESTBED/
scp olsrdAlix root@10.1.129.138:/root/COGNET_TESTBED/
