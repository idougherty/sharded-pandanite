#!/bin/bash

# usage ./test.sh [n] [d]
# 	- n: number of peers
#	- d: delay in seconds(for sleep)
# invokes ./server [n], sleep [d], ./peers [n]
./runserver.sh $1
sleep $2
./peers.sh $1


