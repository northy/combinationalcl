#include <iostream>
#include <parser.hpp>
#include <compiler.hpp>

#include <unistd.h>
#include <ios>
#include <fstream>
#include <string>
#include <ctime>

void mem_usage(double& vm_usage, double& resident_set) {
   vm_usage = 0.0;
   resident_set = 0.0;
   std::ifstream stat_stream("/proc/self/stat",std::ios_base::in);
   std::string pid, comm, state, ppid, pgrp, session, tty_nr;
   std::string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   std::string utime, stime, cutime, cstime, priority, nice;
   std::string O, itrealvalue, starttime;
   unsigned long vsize;
   long rss;
   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
   >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
   >> utime >> stime >> cutime >> cstime >> priority >> nice
   >> O >> itrealvalue >> starttime >> vsize >> rss;
   stat_stream.close();
   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;
   vm_usage = vsize / 1024.0;
   resident_set = rss * page_size_kb;
}

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(0); std::cout.tie(0); std::cin.tie(0);

    if (argc<2) return 1;

    using std::cout;

    std::clock_t start;

    start = std::clock();

    auto result = parser::parsevcd::parse_vcd_file(argv[1]);

    if(result) {
        cout << "Parser OK!\n";
        double vm, rss;
        mem_usage(vm, rss);
        cout << "Virtual Memory: " << vm << "\nResident set size: " << rss << '\n';
        cout << "Execution time: " << ( std::clock() - start ) / (double) CLOCKS_PER_SEC << " seconds\n";
        cout << "Date: " << result->date;
        cout << "Version: " << result->version;
        cout << "Comment: " << result->comment.value_or("404\n");
        cout << "Timescale: " << result->timescale << '\n';
        cout << "Scopes:\n";
        for (auto &s:result->scope) {
            cout << s << '\n';
        }
        cout << "Signals:\n";
        for (auto &s:result->signals) {
            cout << "Name: " << s.name << " | ID: " << s.id << " | Type: " << s.type << " | Bit width: " << s.bitwidth << '\n';
        }
        cout << "Initial Dump:\n";
        for (auto &d : result->initial_dump) {
            cout << d.id << ' ';
            for (auto &v:d.value) {
                cout << v;
            }
            cout << '\n';
        }
        cout << "Timestamps:\n";
        for (auto &t : result->timestamps) {
            cout << '#' << t.time << '\n';
            for (auto &d : t.dumps) {
                cout << d.id << ' ';
                for (auto &v:d.value) {
                    cout << v;
                }
                cout << '\n';
            }
        }

        std::ofstream out("out.vcd");
        if (out.is_open()) {
            cout << "Compiling file...\n";
            compiler::compilevcd::compile_vcd_file(result.value(),out);
            out.close();
        }
    }
    else {
        cout << "Parser ERROR\n";
    }
}
