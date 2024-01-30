#!/bin/sh
gcc -o libnop.so nop.c -fPIC -shared -Os
gcc -o prog prog.c
./prog

