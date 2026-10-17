#!/bin/bash
#----------------------------------------------------------------
# Fetch from website http://www.alglib.net/
#wget -c http://www.alglib.net/translator/re/alglib-2.6.0.cpp.zip
unzip alglib-2.6.0.cpp.zip
mv cpp alglib
cd alglib
chmod u+x build
./build gcc "-pipe -std=c++17 -O2 -ftree-loop-vectorize -Wall -Wextra -Wpedantic -Werror=format-security -D_GLIBC_ASSERTIONS -fasynchronous-unwind-tables -fexceptions -fpie -fstack-protector-strong"
