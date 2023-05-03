/************************************************************************/
/* TimeWheel实现了一个毫秒级别的定时器，最大支持到分钟级别                                                                     */
/************************************************************************/

#pragma once
#include<functional>
#include<list>
#include<thread>
#include<mutex>
#include "timer.h"

typedef struct TimePos_
{
	int ms_pos;
	int s_pos;
	int min_pos;
}TimePos;

typedef struct EventInfo_
{
	int interval;
	std::function<void(void)> call_back;
	TimePos time_pos;
	int timer_id;

}EventInfo;

class TimeWheel
{
public:
	TimeWheel();
	~TimeWheel();
public:
	/*step 以毫秒为单位，表示定时器最小时间粒度
	 *max_timer 表示定时器所能接受的分钟时间间隔
	 */
	bool InitTimerWheel(int step,int max_min);
	int AddTimer(int interval, std::function<void(void)>& call_back);
	void DeleteTimer(int timer_id);

private:
	void DoLoop();
	int GenerateTimerID();
	bool InsertTimer(int diff_ms,EventInfo& einfo);
	bool GetNextTrigerPos(int interval,TimePos& time_pos);
	int GetMS(TimePos time_pos);
	bool DealTimeWheeling(std::list<EventInfo> leinfo);
private:
	std::unique_ptr<std::list<EventInfo>[]> m_callbackList;
	std::mutex m_mutex;
	TimePos m_timePos;
	int m_lowCount;
	int m_midCount;
	int m_highCount;
	int m_stepMs;
	int m_timerCount;
	Timer m_timer;

};

