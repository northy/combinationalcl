#!/usr/bin/env sh

if [ $# -lt 4 ]
then
    echo "Usage: ./run.sh <ports count> <inputs count> <outputs count> <events count>"
    exit 1
fi

../rust/target/debug/combinationalcl $1 $2 $3 $4
if [ $? -ne 0 ]
then
    echo "An error has occured"
    exit 1
fi

../cpp/build/CombinationaCL $1 $2 $3 out.cl out.vcd
if [ $? -ne 0 ]
then
    echo "An error has occured"
    exit 1
fi
