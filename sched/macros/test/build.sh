#!/bin/bash

g++ ../../sched.cpp ../jump_table.cpp  test.cpp $1 -I../../.. -o test -ggdb
