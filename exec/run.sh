#!/usr/bin/env sh

if [ $# -lt 3 ]
then
    echo "Usage: ./run.sh <ports count> <inputs count> <outputs count>"
    exit 1
fi

../rust/target/debug/combinationalcl $1 $2 $3
if [ $? -ne 0 ]
then
    echo "An error has occured"
    exit 1
fi

../cpp/build/CombinationaCL $2 $3 out.cl out.vcd
if [ $? -ne 0 ]
then
    echo "An error has occured"
    exit 1
fi
