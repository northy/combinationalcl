#!/bin/bash
#basic packages
dpkg --add-architecture i386
apt-get update -y && apt upgrade -y
apt-get install awscli python3 git g++ cmake libboost-all-dev ocl-icd-* opencl-headers cargo -y
wget https://raw.githubusercontent.com/KhronosGroup/OpenCL-CLHPP/master/include/CL/opencl.hpp -O /usr/include/CL/opencl.hpp

#opencl drivers
cd /home/ubuntu
aws s3 cp --recursive s3://ec2-amd-linux-drivers/latest/ .
tar -xf amdgpu-pro*ubuntu*.xz
rm *.xz
cd amdgpu-pro*

apt install linux-modules-extra-$(uname -r) -y
cat RPM-GPG-KEY-amdgpu | apt-key add -

./amdgpu-pro-install -y --opencl=pal,legacy --headless --no-dkms
#./amdgpu-pro-install -y --opencl=rocr --headless --no-dkms

usermod -a -G video ubuntu
usermod -a -G render ubuntu

#compile
cd /home/ubuntu
git clone https://github.com/northy/combinationalcl.git
cd combinationalcl
cd exec
bash compile.sh
sudo chown ubuntu /home/ubuntu/combinationalcl/ -R

echo > /home/ubuntu/done
reboot
