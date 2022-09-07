#!/bin/bash

g++ ../sched.cpp test_basic.cpp $1 -I../.. -o test -ggdb
