#!/bin/bash
apt-get update -y
apt-get install git g++ cmake libboost-all-dev ocl-icd-* opencl-headers cargo -y
wget https://raw.githubusercontent.com/KhronosGroup/OpenCL-CLHPP/master/include/CL/opencl.hpp -O /usr/include/CL/opencl.hpp

cd /home/ubuntu
git clone https://github.com/northy/combinationalcl.git
cd combinationalcl
cd exec
bash compile.sh

sudo chown ubuntu /home/ubuntu/combinationalcl/ -R
echo > /home/ubuntu/done
