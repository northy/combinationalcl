#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <tuple>
#include <cmath>

#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include <CL/cl2.hpp>
#define CL_STD "-cl-std=CL2.0"

#include <compiler.hpp>
#include <parser.hpp>
#include <vcd.hpp>
#include <helper.hpp>

#define CPU_GPU_DEF CL_DEVICE_TYPE_ALL

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

    if (argc<6) {
        std::cerr << "./CombinationaCL <ports count> <input count> <output count> <kernel file> <vcd file>" << std::endl;
        return 1;
    }
    int pc=atoi(argv[1]), ic=atoi(argv[2]), oc=atoi(argv[3]);
    std::string kernelpath = argv[4];
    const char* vcdpath = argv[5];

    srand(time(0));

    //START

    //MEMORY OBJECTS

    //Parsing VCD
    auto vcd_ast = parser::parsevcd::parse_vcd_file(vcdpath);
    if (vcd_ast) {
        std::cout << "Success parsing VCD file" << std::endl;
    }
    else {
        std::cerr << "Error while parsing VCD file" << std::endl;
        return 1;
    }

    int runtime = vcd_ast->timestamps.size();

    std::cout << "Ports: " << pc << " Inputs: " << ic << " Outputs: " << oc << " Runtime: " << runtime << std::endl;

    std::vector<char> inputs(runtime*ic,1), ports(runtime*pc), outputs(runtime*oc);

    for (vcd::Timestamp t : vcd_ast->timestamps) {
        for (int i=0; i<t.dumps.size();++i) {
            inputs[t.time*ic+i] = t.dumps[i].value[0]-'0';
        }
    }

    /*for (int i=0; i<runtime*ic; i++) {
        std::cout << (int)inputs[i] << ' ';
        if ((i+1)%ic==0) std::cout << std::endl;
    }*/

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
    cl::Context context(devices, NULL, NULL, NULL, &err);
    if (err) {std::cerr << "Context error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }

    //OPEN PROGRAM
    std::ifstream programFile(kernelpath);
    std::string programSrc(std::istreambuf_iterator<char>(programFile), (std::istreambuf_iterator<char>()));
    
    //CREATE PROGRAM
    cl::Program program(context, programSrc, true, &err);
    if (err) {
        std::cerr << "Program error: " << getErrorString(err) << " (" << err << ")\n";

        std::cerr << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device, &err) << std::endl;

        return 1;
    }

    try {
        err = program.build(CL_STD);
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
        return 1;
    }
    std::cout << "Success building program" << std::endl;

    //COMMAND QUEUE
    cl::CommandQueue queue(context, device, 0, &err);
    if (err) {std::cerr << "Command queue error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }

    //BUFFERS
    cl::Buffer inputs_buffer(context, inputs.begin(), inputs.end(), true);
    if (err) {std::cerr << "Buffer error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }
    cl::Buffer ports_buffer(context, CL_MEM_WRITE_ONLY, ports.size());
    if (err) {std::cerr << "Buffer error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }
    cl::Buffer outputs_buffer(context, CL_MEM_WRITE_ONLY, outputs.size());
    if (err) {std::cerr << "Buffer error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }
    //err = cl::copy(context, std::begin(inputs), std::end(inputs), inputs_buffer);
    if (err) {std::cerr << "Copy error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }
    std::cout << "Success building buffers" << std::endl;

    //KERNEL
    cl::KernelFunctor
    <cl_int, cl::Buffer, cl_int, cl::Buffer, cl_int, cl::Buffer>
    kernelfunctor(program, "combinational");
    cl::Kernel kernel = kernelfunctor.getKernel();

    err = kernel.setArg(0, ic);
    if (err) {std::cerr << "Arg0 error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }
    err = kernel.setArg(1, inputs_buffer);
    if (err) {std::cerr << "Arg1 error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }
    err = kernel.setArg(2, pc);
    if (err) {std::cerr << "Arg2 error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }
    err = kernel.setArg(3, ports_buffer);
    if (err) {std::cerr << "Arg3 error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }
    err = kernel.setArg(4, oc);
    if (err) {std::cerr << "Arg4 error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }
    err = kernel.setArg(5, outputs_buffer);
    if (err) {std::cerr << "Arg5 error: " << getErrorString(err) << " (" << err << ")\n"; return 1; }
    std::cout << "Success building kernel" << std::endl;

    //RESTRICTIONS
    size_t localsz = kernel.getWorkGroupInfo
    <CL_KERNEL_WORK_GROUP_SIZE>(device);
    //cl::NDRange local(localsz,localsz);
    cl::NDRange local(1);
    cl::NDRange global(runtime);

    //ENQUEUE
    std::cout << "Enqueueing..." << std::endl;
    cl::Event event;
    err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local, NULL, &event);
    if (err) {
        std::cerr << "Enqueue range error: " << getErrorString(err) << std::endl;
        return 1;
    }
    event.wait();
    cl::copy(queue, outputs_buffer, outputs.begin(), outputs.end());
    cl::copy(queue, ports_buffer, ports.begin(), ports.end());
    
    //FINISH
    /*for (int i=0; i<runtime*oc; i++) {
        std::cout << (int)outputs[i] << ' ';
        if ((i+1)%oc==0) std::cout << std::endl;
    }*/
    std::cout << "Done\n";

    for (int j=0; j<pc; ++j) {
        std::string name = "p"+std::to_string((int)j);
        std::string id = genid(ic+j);
        vcd_ast->signals.push_back(vcd::Signal{"wire",1,id,name});
        vcd_ast->initial_dump.push_back(vcd::Dump{std::vector<char>{'0'},id});
        for (int i=0; i<runtime; i++) {
            char value = '0'+ports[i*pc+j];
            vcd_ast->timestamps[i].dumps.push_back(vcd::Dump{std::vector<char>{value},id});
        }
    }

    for (int j=0; j<oc; ++j) {
        std::string name = "o"+std::to_string((int)j);
        std::string id = genid(ic+pc+j);
        vcd_ast->signals.push_back(vcd::Signal{"wire",1,id,name});
        vcd_ast->initial_dump.push_back(vcd::Dump{std::vector<char>{'0'},id});
        for (int i=0; i<runtime; i++) {
            char value = '0'+outputs[i*oc+j];
            vcd_ast->timestamps[i].dumps.push_back(vcd::Dump{std::vector<char>{value},id});
        }
    }
    
    std::ofstream out("out_sim.vcd");
    if (out.is_open()) {
        std::cout << "Compiling file...\n";
        compiler::compilevcd::compile_vcd_file(vcd_ast.value(),out);
        out.close();
    }

    return 0;
}
