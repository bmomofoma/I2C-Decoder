#pragma once
#include <vector>
#include "sample.h"

struct Edge {
    uint64_t time_ns;
    enum class Signal { SCL, SDA } signal;
    enum class Direction { RISING, FALLING } direction;
};

std::vector<Edge> detect_edges(const std::vector<Sample>& samples);