#!/bin/bash

g++ ../../sched.cpp ../macros.cpp test1.cpp $1 -I../../.. -o test1 -ggdb
g++ ../../sched.cpp ../macros.cpp test2.cpp $1 -I../../.. -o test2 -ggdb
