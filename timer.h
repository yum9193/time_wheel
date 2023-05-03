#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <signal.h>
#include <time.h>

class Timer {
public:
    Timer(const std::function<void()> &callback);

    void start(int interval_millisec);

    void stop();

private:
    static void timer_handler(int sig, siginfo_t* si, void* uc);

    timer_t timerid_;
    std::function<void()> callback_;
};

