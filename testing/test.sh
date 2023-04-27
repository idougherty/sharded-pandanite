#!/bin/bash

# usage ./test.sh [n] [p] [d]
# 	- n: number of peers
#	- p: number of times to ping 
#	- d: delay in seconds(for sleep)
# invokes ./server [n], sleep [d], ./peers [n]
./runserver.sh $1
sleep 1
./peers.sh $1 $2 $3