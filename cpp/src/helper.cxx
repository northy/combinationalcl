#include <helper.hpp>

std::string genid(int id) {
    std::vector<char> basev;
    for (char i=33; i<127; ++i)
        basev.push_back(i);

    std::string encoded;
    int base = basev.size();

    while (id/base>0) {
        encoded = basev[id%base] + encoded;
        id = id/base;
    }
    encoded = basev[id%base] + encoded;

    return encoded;
}