#include "i2c_decoder.h"
#include <iomanip>
#include <sstream>

const std::vector<std::string>& I2CDecoder::get_log() const {
    return log_;
}

// Formats our output to look like the logic analyzer spec
void I2CDecoder::emit(uint64_t time_ns, const std::string& msg) {
    std::stringstream ss;
    ss << "[" << std::setw(7) << time_ns << " ns] " << msg;
    log_.push_back(ss.str());
}

void I2CDecoder::process_samples(const std::vector<Sample>& samples) {
    if (samples.empty()) return;
    
    // Initialize our tracking variables to the first sample
    prev_scl = samples[0].scl;
    prev_sda = samples[0].sda;
    
    for (const auto& s : samples) {
        handle_sample(s);
    }
}

void I2CDecoder::handle_sample(const Sample& s) {
    bool scl_rising = (!prev_scl && s.scl);
    bool sda_falling = (prev_sda && !s.sda);
    bool sda_rising = (!prev_sda && s.sda);

    bool scl_stable_high = (s.scl && prev_scl);

    // 1. START Condition
    if (sda_falling && scl_stable_high) {
        // ERROR CHECK: Did we get interrupted mid-byte?
        if (state == State::GET_ADDR || state == State::GET_DATA) {
            if (bit_count != 0) {
                emit(s.time_ns, "[ERROR] REPEATED START condition mid-byte! (Bus corrupted)");
            }
        }

        if (state != State::IDLE) {
            emit(s.time_ns, "REPEATED START");
        } else {
            emit(s.time_ns, "START");
        }
        state = State::GET_ADDR;
        bit_count = 0;
        shift_reg = 0;
    }
    // 2. STOP Condition
    else if (sda_rising && scl_stable_high) {
        // ERROR CHECK: Did the master hang up mid-byte?
        if (state == State::GET_ADDR || state == State::GET_DATA) {
            if (bit_count != 0) {
                emit(s.time_ns, "[ERROR] STOP condition mid-byte! (Bus corrupted)");
            }
        }
        
        emit(s.time_ns, "STOP");
        state = State::IDLE;
    }
    // 3. Sample Data
    else if (scl_rising) {
        switch (state) {
            case State::IDLE:
                break; 
                
            case State::GET_ADDR:
                shift_reg = (shift_reg << 1) | (s.sda ? 1 : 0);
                bit_count++;
                if (bit_count == 8) {
                    uint8_t addr = shift_reg >> 1; 
                    std::string rw = (shift_reg & 0x01) ? "read" : "write"; 
                    
                    std::stringstream msg;
                    msg << "ADDR=0x" << std::hex << std::uppercase << (int)addr << " RW=" << rw;
                    emit(s.time_ns, msg.str());
                    
                    state = State::GET_ACK_ADDR;
                }
                break;
                
            case State::GET_ACK_ADDR:
                // ERROR CHECK: Loudly flag NACKs
                if (s.sda) {
                    emit(s.time_ns, "[WARNING] NACK received (Device missing/busy)");
                } else {
                    emit(s.time_ns, "ACK");
                }
                state = State::GET_DATA;
                bit_count = 0;
                shift_reg = 0;
                break;
                
            case State::GET_DATA:
                shift_reg = (shift_reg << 1) | (s.sda ? 1 : 0);
                bit_count++;
                if (bit_count == 8) {
                    std::stringstream msg;
                    msg << "DATA=0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)shift_reg;
                    emit(s.time_ns, msg.str());
                    
                    state = State::GET_ACK_DATA;
                }
                break;
                
            case State::GET_ACK_DATA:
                // ERROR CHECK: Loudly flag NACKs
                if (s.sda) {
                    emit(s.time_ns, "[WARNING] NACK received on data byte");
                } else {
                    emit(s.time_ns, "ACK");
                }
                state = State::GET_DATA;
                bit_count = 0;
                shift_reg = 0;
                break;
        }
    }

    prev_scl = s.scl;
    prev_sda = s.sda;
}