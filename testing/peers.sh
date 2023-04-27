#!/bin/bash

# usage ./peers.sh [n] [p] [d]
#	- n: number of peers, iterates from peer 3000 + 0 to 3000 + n
#	- p: number of times to ping
#	- d: delay in between pings

for p in $(seq 1 $2)
do
	curl -X GET http://localhost:3000/peers >> 3kpeers.txt
	echo >> 3kpeers.txt
	for s in $(seq 1 $1)
	do
		curl -X GET http://localhost:$((3000 + $s))/peers >> server$s/peers.txt
		echo >> server$s/peers.txt
	done
	sleep $3
done
