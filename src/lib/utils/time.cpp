#include "time.hpp"

namespace utils {

std::time_t Time::getCurrentTime() {
    return std::time(NULL);
}

double Time::diffTimeSeconds(std::time_t end, std::time_t start) {
    return std::difftime(end, start);
}

}