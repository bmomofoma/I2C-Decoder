#include "edge_detector.h"

std::vector<Edge> detect_edges(const std::vector<Sample>& samples) {
    std::vector<Edge> edges;
    
    // If we have no data, return an empty list
    if (samples.empty()) return edges;

    // Set our initial state to the very first sample
    bool prev_scl = samples[0].scl;
    bool prev_sda = samples[0].sda;

    // Start checking from the second sample onwards
    for (size_t i = 1; i < samples.size(); ++i) {
        const auto& s = samples[i];

        // Did the SCL line change?
        if (s.scl != prev_scl) {
            Edge e;
            e.time_ns = s.time_ns;
            e.signal = Edge::Signal::SCL;
            e.direction = s.scl ? Edge::Direction::RISING : Edge::Direction::FALLING;
            edges.push_back(e);
            prev_scl = s.scl;
        }

        // Did the SDA line change?
        if (s.sda != prev_sda) {
            Edge e;
            e.time_ns = s.time_ns;
            e.signal = Edge::Signal::SDA;
            e.direction = s.sda ? Edge::Direction::RISING : Edge::Direction::FALLING;
            edges.push_back(e);
            prev_sda = s.sda;
        }
    }

    return edges;
}