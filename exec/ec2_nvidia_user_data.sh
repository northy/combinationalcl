#!/bin/bash
#basic packages
dpkg --add-architecture i386
apt-get update -y && apt upgrade -y
apt-get install awscli python3 git g++ cmake libboost-all-dev ocl-icd-* opencl-headers cargo -y
wget https://raw.githubusercontent.com/KhronosGroup/OpenCL-CLHPP/master/include/CL/opencl.hpp -O /usr/include/CL/opencl.hpp

#opencl drivers
cd /home/ubuntu
sudo apt install linux-headers-$(uname -r)
curl -O https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-ubuntu2004.pin
mv cuda-ubuntu2004.pin /etc/apt/preferences.d/cuda-repository-pin-600
apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/7fa2af80.pub
sudo add-apt-repository "deb https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/ /"
apt update -y
apt -y install cuda-drivers

#compile
cd /home/ubuntu
git clone https://github.com/northy/combinationalcl.git
cd combinationalcl
cd exec
bash compile.sh
sudo chown -R ubuntu /home/ubuntu/combinationalcl/

echo > /home/ubuntu/done
reboot
