#ifndef CHRONOMETER_HPP
#define CHRONOMETER_HPP

#include <chrono>
namespace _2Ls
{
    class Chronometer
    {
        std::chrono::time_point<std::chrono::high_resolution_clock> hi_res_start, hi_res_end;
        std::chrono::time_point<std::chrono::steady_clock> steady_start, steady_end;
        std::chrono::time_point<std::chrono::system_clock> system_start, system_end;

    public:
        void set_hi_res_start() noexcept { hi_res_start = std::chrono::high_resolution_clock::now(); };
        void set_hi_res_end() noexcept { hi_res_end = std::chrono::high_resolution_clock::now(); };
        void set_steady_start() noexcept { steady_start = std::chrono::steady_clock::now(); };
        void set_steady_end() noexcept { steady_end = std::chrono::steady_clock::now(); };
        void set_system_start() noexcept { system_start = std::chrono::system_clock::now(); };
        void set_system_end() noexcept { system_end = std::chrono::system_clock::now(); };
        double get_ns() const noexcept { return std::chrono::duration_cast<std::chrono::nanoseconds>(hi_res_end - hi_res_start).count(); };
        double get_us() const noexcept { return std::chrono::duration_cast<std::chrono::microseconds>(hi_res_end - hi_res_start).count(); };
        double get_ms() const noexcept { return std::chrono::duration_cast<std::chrono::milliseconds>(steady_end - steady_start).count(); };
        double get_s() const noexcept { return std::chrono::duration_cast<std::chrono::seconds>(steady_end - steady_start).count(); };
        double get_min() const noexcept { return std::chrono::duration_cast<std::chrono::minutes>(system_end - system_start).count(); };
        double get_hr() const noexcept { return std::chrono::duration_cast<std::chrono::hours>(system_end - system_start).count(); };
    };
}

#endif
