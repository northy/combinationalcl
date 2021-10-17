#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <tuple>
#include <cmath>
#include <chrono>

#include <boost/lexical_cast.hpp>

#include <definitions.hpp>
#include <compiler.hpp>
#include <parser.hpp>
#include <vcd.hpp>
#include <helper.hpp> //OpenCL included here

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(0); std::cout.tie(0); std::cin.tie(0);
    cl_int err;

    if (argc!=6) {
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

    const int runtime = vcd_ast->timestamps.size();

    std::cout << "Ports: " << pc << " Inputs: " << ic << " Outputs: " << oc << " Runtime: " << runtime << std::endl;
    const double glocalmem = (double)((pc+ic)*runtime)/1024;
    const double gglobalmem = (double)((pc+ic+oc)*runtime)/1024;

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
        std::cerr << "Platform error: " << getErrorString(err) << std::endl;
        return 1;
    }
    std::cout << "Platform count is: " << platformList.size() << std::endl;
    if (platformList.size()<=PLATFORM_NUMBER) { std::cerr << "Platform error: Not enough platforms" << std::endl; return 1; }
    
    cl::Platform plat = platformList[PLATFORM_NUMBER];
    std::cout << "Using " << plat.getInfo<CL_PLATFORM_NAME>() << std::endl;

    //DEVICE
    std::vector<cl::Device> devices;
    plat.getDevices(CPU_GPU_DEF, &devices);
    std::cout << "Device count is: " << devices.size() << std::endl;

    if (devices.size()<=DEVICE_NUMBER) { std::cerr << "Device error: Not enough devices" << std::endl; return 1; }
    
    cl::Device device = devices[DEVICE_NUMBER];
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
    cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
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
    cl::NDRange local;
    cl::NDRange global(runtime);

    //ENQUEUE
    std::cout << "Enqueueing..." << std::endl;
    cl::Event event;
    std::chrono::steady_clock::time_point cstart = std::chrono::steady_clock::now();
    err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local, NULL, &event);
    if (err) {
        std::cerr << "Enqueue range error: " << getErrorString(err) << std::endl;
        return 1;
    }
    int cphysmem = pMemGetValue();
    int cvirtmem = vMemGetValue();
    event.wait();
    std::chrono::steady_clock::time_point cend = std::chrono::steady_clock::now();
    cl::copy(queue, outputs_buffer, outputs.begin(), outputs.end());
    cl::copy(queue, ports_buffer, ports.begin(), ports.end());

    auto gend = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();
    auto gstart = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();

    const long unsigned int cpu_time = std::chrono::duration_cast<std::chrono::nanoseconds>(cend - cstart).count(), gpu_time = gend - gstart;
    
    //FINISH
    /*for (int i=0; i<runtime*oc; i++) {
        std::cout << (int)outputs[i] << ' ';
        if ((i+1)%oc==0) std::cout << std::endl;
    }*/
    std::cout << "Done\n";

    #if SIMULATION_MODE
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
    #endif

    std::ofstream log("log.csv", std::ios_base::app | std::ios_base::out);

    log << pc << ',' << ic << ',' << oc << ',' << runtime << ',' << cpu_time << ',' << gpu_time << ',' << cvirtmem << ',' << cphysmem << ',' << boost::lexical_cast<std::string>(glocalmem) << ',' << boost::lexical_cast<std::string>(gglobalmem) << std::endl;

    return 0;
}
