#!/usr/bin/env sh

cd ../cpp
rm -rf build
mkdir -p build
cd build
cmake ..
make -j

cd ../../rust
cargo build
