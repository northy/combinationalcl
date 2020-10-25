#pragma once

#include <vcd.hpp>

namespace compiler {
    namespace compilevcd{
        void compile_vcd_file(const vcd::Vcd& vcd, std::ostream& file);
    }
}