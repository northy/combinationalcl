#include <vcd.hpp>

#include <ostream>
#include <string>

namespace compiler {
    namespace compilevcd{
        void compile_vcd_file(const vcd::Vcd& vcd, std::ostream& file) {
            file << "$date " << vcd.date << " $end" << '\n';
            file << "$version " << vcd.version << " $end" << '\n';
            file << "$timescale " << vcd.timescale << " $end" << '\n';
            for (std::string s:vcd.scope)
                file << "$scope " << s << " $end" << '\n';
            for (vcd::Signal s:vcd.signals) {
                file << "$var " << s.type << ' ' << s.bitwidth << ' ' << s.id << ' ' << s.name << " $end" << '\n';
            }
            file << "$upscope $end\n$enddefinitions $end\n$dumpvars\n";
            for (vcd::Dump d:vcd.initial_dump) {
                if (d.value.size()>1) file << 'b';
                for (char c:d.value) file << c;
                if (d.value.size()>1) file << ' ';
                file << d.id << '\n';
            }
            file << "$end\n";
            for (vcd::Timestamp t:vcd.timestamps) {
                file << '#' << t.time << '\n';
                for (vcd::Dump d:t.dumps) {
                    if (d.value.size()>1) file << 'b';
                    for (char c:d.value) file << c;
                    if (d.value.size()>1) file << ' ';
                    file << d.id << '\n';
                }
            }
        }
    }
}