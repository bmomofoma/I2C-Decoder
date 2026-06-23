#include "csv_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

std::vector<Sample> load_csv(const std::string& path) {
    std::vector<Sample> samples;
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }

    std::string line;
    
    // Skip the header row
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string time_str, scl_str, sda_str;

        std::getline(ss, time_str, ',');
        std::getline(ss, scl_str, ',');
        std::getline(ss, sda_str, ',');

        Sample s;
        s.time_ns = std::stoull(time_str);
        
        // PULL-UP RESISTOR LOGIC: 
        // Treat both '1' and 'z'/'Z' (High-Z) as a logical HIGH (true).
        s.scl = (scl_str == "1" || scl_str == "z" || scl_str == "Z");
        s.sda = (sda_str == "1" || sda_str == "z" || sda_str == "Z");

        // DELTA CYCLE FILTER:
        // If this sample has the exact same timestamp as the previous one,
        // it means the simulator is evaluating intermediate logic. 
        // Overwrite the previous state instead of adding a new event.
        if (!samples.empty() && samples.back().time_ns == s.time_ns) {
            samples.back().scl = s.scl;
            samples.back().sda = s.sda;
        } else {
            // Otherwise, it's a new nanosecond step, so add it to the timeline.
            samples.push_back(s);
        }
    }

    return samples;
}