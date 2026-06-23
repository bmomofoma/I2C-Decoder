#pragma once
#include <vector>
#include <string>
#include "sample.h"

class I2CDecoder {
public:
    void process_samples(const std::vector<Sample>& samples);
    const std::vector<std::string>& get_log() const;

private:
    enum class State { IDLE, GET_ADDR, GET_ACK_ADDR, GET_DATA, GET_ACK_DATA };
    State state = State::IDLE;
    
    uint8_t bit_count = 0;
    uint8_t shift_reg = 0;
    
    bool prev_scl = true;
    bool prev_sda = true;
    
    std::vector<std::string> log_;

    void handle_sample(const Sample& s);
    void emit(uint64_t time_ns, const std::string& msg);
};