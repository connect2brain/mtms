//
// Created by Kalle Jyrkinen on 16.11.2021.
//

#ifndef TMS_CPP_TIMER_HPP
#define TMS_CPP_TIMER_HPP

#include <iostream>
#include <chrono>


class Timer {
public:
    Timer() : start_time_(std::chrono::time_point<std::chrono::steady_clock>::max()) {}

    void Start() { start_time_ = std::chrono::steady_clock::now(); }

    void Elapsed() {
        auto stop_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time_);
        if (stop_time < start_time_)
            std::cout << "Timer has not been started." << std::endl;
        else
            std::cout << "Time elapsed: " << (float) duration.count() / 1000 << " seconds." << std::endl;
    }
    void Elapsed(int32_t N) {
        auto stop_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time_);
        if (stop_time < start_time_)
            std::cout << "Timer has not been started." << std::endl;
        else
            std::cout << "Seconds per iteration: " << (float) duration.count() / (1000 * N) << " s" << std::endl;
    }

private:
    std::chrono::time_point<std::chrono::steady_clock> start_time_;

};


#endif //TMS_CPP_TIMER_HPP
