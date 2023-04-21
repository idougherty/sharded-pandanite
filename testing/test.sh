#!/bin/bash

# usage ./test.sh [n]
#     - n: number of servers to start
# Start root node guy
cd ..
./bin/server --local -p 3000 &
cd testing

# create n servers
for s in $(seq 1 $1)
do
	mkdir server$s
	cd server$s
	cp ../../genesis.json .
	../../bin/keygen && ../../bin/server --local -p 300$s --host-source http://localhost:3000 &
	cd ..
done
