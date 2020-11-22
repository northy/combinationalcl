#!/usr/bin/env sh

cd ../cpp
mkdir -p build
cd build
cmake ..
make -j

cd ../../rust
cargo build
