#!/bin/bash
gcc	-c	lab4_1.c
gcc	-o	lab4_1	lab4_1.o		-lpthread
gcc	-c	lab4_2.c
gcc	-o	lab4_2	lab4_2.o		-lpthread
export PATH="./"
