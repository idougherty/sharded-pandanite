#!/bin/bash

# usage ./tx.sh [pubKey] [privKey] [to] [amount] [n] [port]
#     - pubKey: in key.json			(required)
#     - privKey: in key.json		(required)
#     - to: destination wallet 		(default = NULL)
#     - amount: money				(default = 1)
#     - n: number of transactions	(default = 1)
#     - port: server to send post	(default = 3000)
#
# 		******* Use -1 to specify default *******

PUBKEY=$1
PRIVKEY=$2
TO=$3
AMOUNT=$4
N=$5
PORT=$6

if [ -z "$PUBKEY" ]
  then
	echo "Error: I need a pubkey & privkey."
	exit 1
fi

if [ -z "$PRIVKEY" ]
  then
	echo "Error: I need a pubkey & privkey."
	exit 1
fi

if [ -z "$3"  ] || [ "$3" == -1 ]
  then
	TO=00000000000000000000000000000000000000000000000000
fi

if [ -z "$4" ] || [ "$4" == -1 ]
  then
	AMOUNT=1
fi

if [ -z "$5" ] || [ "$5" == -1 ]
  then
	N=1
fi

if [ -z "$6" ] || [ "$6" == -1 ]
  then
	PORT=3000
fi

cd ..
# create n transactions
for n in $(seq 1 $N)
do
	TX=$(./bin/tx $PUBKEY $PRIVKEY $TO $AMOUNT 0 $RANDOM)

	echo $TX

	curl -X POST -H "Content-Type: application/json" -d "[$TX]" http://localhost:$PORT/add_transaction_json
done
