
#ifndef TMS_CPP_TIMER_HPP
#define TMS_CPP_TIMER_HPP

#include <iostream>
#include <chrono>

/**
 * @brief Helper class for measuring performance of the library
 * 
 */
class Timer {
public:
    /**
     * @brief Construct a new Timer object
     * 
     */
    Timer() : start_time_(std::chrono::time_point<std::chrono::steady_clock>::max()) {}

    /**
     * @brief Start timing. This can be called multiple times on same instance of timer class to do multiple measurements
     * 
     */
    void Start() { start_time_ = std::chrono::steady_clock::now(); }

    /**
     * @brief Stop timing and print the result
     * 
     */
    void Elapsed() {
        auto stop_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time_);
        if (stop_time < start_time_)
            std::cout << "Timer has not been started." << std::endl;
        else
            std::cout << "Time elapsed: " << (float) duration.count() / 1000 << " seconds." << std::endl;
            
        start_time_ = std::chrono::time_point<std::chrono::steady_clock>::max();
    }
    /**
     * @brief Stop timing and print the result assuming averaging over N iterations
     * 
     */
    void Elapsed(int32_t N) {
        auto stop_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time_);
        if (stop_time < start_time_)
            std::cout << "Timer has not been started." << std::endl;
        else
            std::cout << "Seconds per iteration: " << (float) duration.count() / (1000 * N) << " s" << std::endl;
        
        start_time_ = std::chrono::time_point<std::chrono::steady_clock>::max();
    }
    /**
     * @brief Return the current time (in seconds)
     * 
     */
    float GetElapsed() {
        auto stop_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time_);
        float elapsed = 0;
        if (stop_time < start_time_)
            std::cout << "Timer has not been started." << std::endl;
        else
            elapsed = duration.count() / 1000.0 ;
        
        start_time_ = std::chrono::time_point<std::chrono::steady_clock>::max();
        return elapsed;
    }
private:
    std::chrono::time_point<std::chrono::steady_clock> start_time_;

};


#endif //TMS_CPP_TIMER_HPP
