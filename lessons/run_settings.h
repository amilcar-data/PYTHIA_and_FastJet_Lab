#ifndef PYTHIA_LAB_RUN_SETTINGS_H
#define PYTHIA_LAB_RUN_SETTINGS_H

#include <iostream>
#include <limits>
#include <string>

// Reject invalid counts before initialization or overwriting any output files.
inline bool readEventCount(int argc, char* argv[], int fallback, int& count) {
    count = fallback;
    if (argc == 1) return true;
    if (argc == 2) {
        try {
            const std::string text(argv[1]);
            std::size_t used = 0;
            const long long value = std::stoll(text, &used);
            if (used == text.size() && value > 0
                && value <= std::numeric_limits<int>::max()) {
                count = static_cast<int>(value);
                return true;
            }
        } catch (const std::exception&) {}
    }
    std::cerr << "Usage: " << argv[0] << " [positive integer event count]\n";
    return false;
}
#endif
