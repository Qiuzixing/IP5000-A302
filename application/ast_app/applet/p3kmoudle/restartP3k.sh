#!/bin/bash
ID=`ps|grep "p3ktcp"|grep -v grep|awk '{print $1}'`
echo $ID

for id in $ID
do
kill -9 $id
echo "killed $id"
done
sleep 1
./p3ktcp
