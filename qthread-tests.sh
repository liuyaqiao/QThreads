#!/bin/sh

file_args=qthread.c stack.c switch.s

# Compiles test1.c
gcc -g -Wall test1.c $file_args -o tests

# Compiles server.c
gcc -g -Wall server.c $file_args -o serv
