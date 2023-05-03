#include "timer.h"

Timer::Timer(const std::function<void()> &callback) :callback_(callback) {}

void Timer::start(int interval_millisec) {
	struct sigevent sev;
	struct itimerspec its;
	struct sigaction sa;

	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = &Timer::timer_handler;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGRTMIN, &sa, NULL);

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGRTMIN;
	sev.sigev_value.sival_ptr = &timerid_;
	timer_create(CLOCK_REALTIME, &sev, &timerid_);

	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = interval_millisec * 1000000;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = interval_millisec * 1000000;
	timer_settime(timerid_, 0, &its, NULL);
}

void Timer::stop() {
	timer_delete(timerid_);
}

void Timer::timer_handler(int sig, siginfo_t* si, void* uc) {
	Timer *timer = static_cast<Timer *>(si->si_value.sival_ptr);
	timer->callback_();
}


