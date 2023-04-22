#!/bin/bash

# usage ./server.sh [n]
#	- n: number of servers to start
cd..
./bin/server --local -ip http://localhost -p 3000 >> ./testing/3kout.txt &
cd testing

# create n servers
for s in $(seq 1 $1)
do
	mkdir server$s
	cd server$s
	cp ../../genesis.json .
	../../bin/keygen && ../../bin/server --local -ip http://localhost -p $((3000 + $s)) >> out.txt &
	cd ..
done
