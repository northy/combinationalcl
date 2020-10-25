#pragma once

#include <optional>
#include <vcd.hpp>

namespace parser {
    namespace parsevcd{
        std::optional<vcd::Vcd> parse_vcd_file(const char* filename);
    }
}