#pragma once
#include <stdint.h>

struct Sample {
    uint64_t time_ns;
    bool scl;
    bool sda;
};