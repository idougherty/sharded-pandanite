#!/bin/bash

# usage ./runserver.sh [n]
#	- n: number of servers to start

cd ..
./bin/server --local -ip http://localhost -p 3000 >> ./testing/3kout.txt &
./bin/miner --local >> ./testing/minerout.txt &
cd testing

# create n servers
for s in $(seq 1 $1)
do
	mkdir server$s
	cd server$s
	cp ../../genesis.json .
	../../bin/keygen >> /dev/null
	../../bin/server --local -ip http://localhost -p $((3000 + $s)) >> out.txt &
	../../bin/miner --local -ip http://localhost:$((3000 + $s)) >> minerout.txt &
	cd ..
done
