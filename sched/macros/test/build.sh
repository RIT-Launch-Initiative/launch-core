#!/bin/bash

g++ ../../sched.cpp ../macros.cpp test.cpp $1 -I../../.. -o test -ggdb
