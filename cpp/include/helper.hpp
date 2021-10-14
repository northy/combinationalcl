#pragma once

#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include <CL/opencl.hpp>
#define CL_STD "-cl-std=CL2.0"

#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstring>

std::string genid(int id);
int vMemGetValue();
int pMemGetValue();
const char *getErrorString(cl_int error);
char to_char(short s);
