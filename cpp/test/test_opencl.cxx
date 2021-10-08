#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <tuple>
#include <cmath>

#define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/opencl.hpp>

#include <compiler.hpp>
#include <vcd.hpp>

#define CPU_GPU_DEF CL_DEVICE_TYPE_CPU

char to_char(short s) {
    char c = '0'+s;
    return c;
}

const char *getErrorString(cl_int error)
{
switch(error){
    // run-time and JIT compiler errors
    case 0: return "CL_SUCCESS";
    case -1: return "CL_DEVICE_NOT_FOUND";
    case -2: return "CL_DEVICE_NOT_AVAILABLE";
    case -3: return "CL_COMPILER_NOT_AVAILABLE";
    case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5: return "CL_OUT_OF_RESOURCES";
    case -6: return "CL_OUT_OF_HOST_MEMORY";
    case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8: return "CL_MEM_COPY_OVERLAP";
    case -9: return "CL_IMAGE_FORMAT_MISMATCH";
    case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11: return "CL_BUILD_PROGRAM_FAILURE";
    case -12: return "CL_MAP_FAILURE";
    case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15: return "CL_COMPILE_PROGRAM_FAILURE";
    case -16: return "CL_LINKER_NOT_AVAILABLE";
    case -17: return "CL_LINK_PROGRAM_FAILURE";
    case -18: return "CL_DEVICE_PARTITION_FAILED";
    case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30: return "CL_INVALID_VALUE";
    case -31: return "CL_INVALID_DEVICE_TYPE";
    case -32: return "CL_INVALID_PLATFORM";
    case -33: return "CL_INVALID_DEVICE";
    case -34: return "CL_INVALID_CONTEXT";
    case -35: return "CL_INVALID_QUEUE_PROPERTIES";
    case -36: return "CL_INVALID_COMMAND_QUEUE";
    case -37: return "CL_INVALID_HOST_PTR";
    case -38: return "CL_INVALID_MEM_OBJECT";
    case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40: return "CL_INVALID_IMAGE_SIZE";
    case -41: return "CL_INVALID_SAMPLER";
    case -42: return "CL_INVALID_BINARY";
    case -43: return "CL_INVALID_BUILD_OPTIONS";
    case -44: return "CL_INVALID_PROGRAM";
    case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46: return "CL_INVALID_KERNEL_NAME";
    case -47: return "CL_INVALID_KERNEL_DEFINITION";
    case -48: return "CL_INVALID_KERNEL";
    case -49: return "CL_INVALID_ARG_INDEX";
    case -50: return "CL_INVALID_ARG_VALUE";
    case -51: return "CL_INVALID_ARG_SIZE";
    case -52: return "CL_INVALID_KERNEL_ARGS";
    case -53: return "CL_INVALID_WORK_DIMENSION";
    case -54: return "CL_INVALID_WORK_GROUP_SIZE";
    case -55: return "CL_INVALID_WORK_ITEM_SIZE";
    case -56: return "CL_INVALID_GLOBAL_OFFSET";
    case -57: return "CL_INVALID_EVENT_WAIT_LIST";
    case -58: return "CL_INVALID_EVENT";
    case -59: return "CL_INVALID_OPERATION";
    case -60: return "CL_INVALID_GL_OBJECT";
    case -61: return "CL_INVALID_BUFFER_SIZE";
    case -62: return "CL_INVALID_MIP_LEVEL";
    case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64: return "CL_INVALID_PROPERTY";
    case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66: return "CL_INVALID_COMPILER_OPTIONS";
    case -67: return "CL_INVALID_LINKER_OPTIONS";
    case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default: return "Unknown OpenCL error";
    }
}
 
int main(int argc, char** argv) {
    std::ios::sync_with_stdio(0); std::cout.tie(0); std::cin.tie(0);
    cl_int err;

    if (argc<3) return 1;
    int ic=atoi(argv[1]), oc=atoi(argv[2]);

    std::cout << "ic: " << ic << " oc: " << oc << std::endl;

    int runtime=8;

    srand(time(0));

    //START

    //PLATFORM
    cl::vector<cl::Platform> platformList;
    err = cl::Platform::get(&platformList);
    if (err) {
        std::cerr << getErrorString(err) << std::endl;
        return 1;
    }
    std::cout << "Platform count is: " << platformList.size() << std::endl;
    cl::Platform plat = platformList[0];
    std::cout << "Using " << plat.getInfo<CL_PLATFORM_NAME>() << std::endl;

    //DEVICE
    std::vector<cl::Device> devices;
    plat.getDevices(CPU_GPU_DEF, &devices);
    auto device = devices.front();

    std::cout << "Using device " << device.getInfo<CL_DEVICE_NAME>() << std::endl;

    //CONTEXT
    cl::Context context(device);

    //OPEN PROGRAM
    std::ifstream programFile("test_kernel.cl");
    std::string programSrc(std::istreambuf_iterator<char>(programFile), (std::istreambuf_iterator<char>()));
    
    //CREATE PROGRAM
    cl::Program program(context, programSrc);

    try {
        err = program.build("-cl-std=CL2.0");
    }
    catch (...) {
        // Print build info for all devices
        cl_int buildErr = CL_SUCCESS;
        auto buildInfo = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(&buildErr);
        for (auto &pair : buildInfo) {
            std::cerr << pair.second << std::endl << std::endl;
        }
        return 1;
    }
    if (err) {
        cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(device);

        // Get the build log
        std::string name     = device.getInfo<CL_DEVICE_NAME>();
        std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
        std::cerr << "Build log for " << name << ":" << std::endl
                    << buildlog << std::endl;

        std::cerr << getErrorString(err) << std::endl;
    }
    std::cout << "Success building program" << std::endl;

    //COMMAND QUEUE
    std::cout << "Enqueueing..." << std::endl;
    cl::CommandQueue queue(context, device, 0, &err);

    //BUFFERS
    std::vector<char> inputs(runtime*ic,1), outputs(runtime*oc);

    cl::Buffer inputs_buffer(std::begin(inputs), std::end(inputs), 1, true, &err);
    if (err) {std::cerr << "Buffer error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }
    cl::Buffer outputs_buffer(std::begin(outputs), std::end(outputs), 0, true, &err);
    if (err) {std::cerr << "Buffer error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }
    err = cl::copy(std::begin(inputs), std::end(inputs), inputs_buffer);
    if (err) {std::cerr << "Copy error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }
    std::cout << "Success building buffers" << std::endl;

    //KERNEL
    cl::KernelFunctor
    <cl_int, cl::Buffer, cl_int, cl::Buffer>
    kernelfunctor(program, "combinational");
    cl::Kernel kernel = kernelfunctor.getKernel();
    kernel.setArg(0, ic);
    kernel.setArg(1, inputs_buffer);
    kernel.setArg(2, oc);
    kernel.setArg(3, outputs_buffer);
    std::cout << "Success building kernel" << std::endl;

    //RESTRICTIONS
    size_t localsz = kernel.getWorkGroupInfo
    <CL_KERNEL_WORK_GROUP_SIZE>(device);
    cl::NDRange local(localsz,localsz);
    cl::NDRange global(runtime, ic);

    //ENQUEUE

    cl::Event event;
    err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NDRange(1,1), NULL, &event);
    std::cout << getErrorString(err) << std::endl;
    event.wait();
    cl::copy(outputs_buffer, std::begin(outputs), std::end(outputs));

    //FINISH
    for (int i=0; i<runtime*oc; i++) {
        std::cout << (int)outputs[i] << ' ';
        if ((i+1)%ic==0) std::cout << std::endl;
    }
    std::cout << "Done\n";

    return 0;
}
