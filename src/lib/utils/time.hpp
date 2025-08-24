#ifndef TIME_HPP
#define TIME_HPP

#include <ctime>

namespace utils {

    class Time {
    public:
        static std::time_t getCurrentTime();
        static double diffTimeSeconds(std::time_t end, std::time_t start);
    };

}

#endif
