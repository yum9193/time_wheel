#include <iostream>
#include <functional>
#include "time_wheel.h"

using namespace std;

void fun100()
{
	timespec ts1;
	clock_gettime(CLOCK_REALTIME, &ts1);
	std::cout << "timestamp10ms " << ts1.tv_sec << "." << ts1.tv_nsec << std::endl;
}
void fun200()
{
	timespec ts1;
	clock_gettime(CLOCK_REALTIME, &ts1);
	std::cout << "timestamp20ms " << ts1.tv_sec << "." << ts1.tv_nsec << std::endl;
}
void fun500()
{
	timespec ts1;
	clock_gettime(CLOCK_REALTIME, &ts1);
	std::cout << "timestamp05ms " << ts1.tv_sec << "." << ts1.tv_nsec << std::endl;
}


int main()
{	
	std::function<void(void)> f100 = std::bind(&fun100);
	std::function<void(void)> f200 = std::bind(&fun200);
	std::function<void(void)> f500 = std::bind(&fun500);

	TimeWheel time_wheel;
	time_wheel.InitTimerWheel(5, 1);
	int timer1 = time_wheel.AddTimer(10, f100);
	int timer2 = time_wheel.AddTimer(20, f200);
	int timer3 = time_wheel.AddTimer(5, f500);

	while (1);
	time_wheel.DeleteTimer(timer1);
	time_wheel.DeleteTimer(timer2);
	time_wheel.DeleteTimer(timer3);
	return 0;	
}
