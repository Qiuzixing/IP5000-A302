#!/bin/sh

CNT=0
TEST_CNT=100

while [ $CNT -lt $TEST_CNT ]; do
        ./start.sh
        CNT=$(( $CNT+1 ))
        echo "CNT=$CNT"
done



