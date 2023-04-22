#!/bin/bash

# usage ./peers.sh [n]
#	- n: number of peers, iterates from peer 3000 + 0 to 3000 + n


curl -X GET http://localhost:3000/peers >> 3kpeers.txt
for s in $(seq 1 $1)
do
	curl -X GET http://localhost:$((3000 + $s))/peers >> server$s/peers.txt
done
