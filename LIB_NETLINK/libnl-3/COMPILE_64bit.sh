#!/usr/bin
make clean
./configure --prefix=$(pwd)/OUTPUT_NL_3_64bit
make
make install
