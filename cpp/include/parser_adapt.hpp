#pragma once

#include <vcd.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(vcd::Signal, type, bitwidth, id, name);
BOOST_FUSION_ADAPT_STRUCT(vcd::Dump, value, id);
BOOST_FUSION_ADAPT_STRUCT(vcd::Timestamp, time, dumps);
BOOST_FUSION_ADAPT_STRUCT(vcd::Vcd, date, version, comment, timescale, scope, signals, initial_dump, timestamps);