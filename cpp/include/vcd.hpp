#pragma once

#include <vector>
#include <string>
#include <optional>

namespace vcd {
    struct Signal {
        std::string type;
        int bitwidth;
        std::string id;
        std::string name;
    };

    struct Dump {
        std::vector<char> value;
        std::string id;
    };

    struct Timestamp {
        uint64_t time;
        std::vector<Dump> dumps;
    };

    struct Vcd {
        std::string date;
        std::string version;
        std::optional<std::string> comment;
        std::string timescale;
        std::vector<std::string> scope;
        std::vector<Signal> signals;
        std::vector<Dump> initial_dump;
        std::vector<Timestamp> timestamps;
    };
}