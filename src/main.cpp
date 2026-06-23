#include <iostream>
#include "csv_parser.h"
#include "i2c_decoder.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input.csv>\n";
        return 1;
    }

    try {
        std::vector<Sample> samples = load_csv(argv[1]);
        
        I2CDecoder decoder;
        decoder.process_samples(samples);
        
        // Print the decoded transaction log
        for (const std::string& log_entry : decoder.get_log()) {
            std::cout << log_entry << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}