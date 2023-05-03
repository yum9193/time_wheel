#include "time_wheel.h"
#include <iostream>
#include <unistd.h>
#include <cstring>

using namespace std;

TimeWheel::TimeWheel() : m_callbackList(nullptr),
						 m_lowCount(0),
						 m_midCount(0),
						 m_highCount(0),
						 m_stepMs(0),
						 m_timerCount(0),
						 m_timePos({0, 0, 0}),
						 m_timer(std::bind(&TimeWheel::DoLoop,this))
 {}


TimeWheel::~TimeWheel()
{
}

bool TimeWheel::InitTimerWheel(int step_ms, int max_min)
{
	if (1000 % step_ms != 0)
	{
		cout << "step is not property, should be devided by 1000" << endl;
		return false;
	}
	m_lowCount = 1000 / step_ms;
	m_midCount = 60;
	m_highCount = max_min;
	m_stepMs = step_ms;

	m_callbackList = std::make_unique<std::list<EventInfo>[]>(m_lowCount + m_midCount + m_highCount);

	m_timer.start(m_stepMs);
	
	return true;
}

int TimeWheel::AddTimer(int interval, std::function<void(void)>& call_back)
{
	if (interval < m_stepMs || interval % m_stepMs != 0 || interval >= m_stepMs * m_lowCount * m_midCount * m_highCount)
	{
		cout << "time interval is invalid" << endl;
		return -1;
	}

	std::unique_lock<std::mutex> lock(m_mutex);

	EventInfo einfo = {0};
	einfo.interval = interval;
	einfo.call_back = call_back;
	einfo.time_pos.ms_pos = m_timePos.ms_pos;
	einfo.time_pos.s_pos = m_timePos.s_pos;
	einfo.time_pos.min_pos = m_timePos.min_pos;
	einfo.timer_id = GenerateTimerID();
	 
	InsertTimer(einfo.interval,einfo);

	m_timerCount++;

	cout << "insert timer success time_id: " << einfo.timer_id << endl;
	return einfo.timer_id;
}
void TimeWheel::DeleteTimer(int time_id)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	int i = 0;
	int nCount = m_lowCount + m_midCount + m_highCount;
	for (i = 0; i < nCount; i++)
	{
		std::list<EventInfo>& leinfo = m_callbackList[i];
		for (auto item = leinfo.begin(); item != leinfo.end();item++)
		{
			if (item->timer_id == time_id)
			{
				item = leinfo.erase(item);
			}
		}
	}

}
void TimeWheel::DoLoop()
{

		std::unique_lock<std::mutex> lock(m_mutex);
		TimePos pos = {0};
		TimePos last_pos = m_timePos;
		GetNextTrigerPos(m_stepMs, pos);
		m_timePos = pos;

		if (pos.min_pos != last_pos.min_pos)
		{
			list<EventInfo>& leinfo =m_callbackList[m_timePos.min_pos + m_midCount + m_lowCount];
			DealTimeWheeling(leinfo);
			leinfo.clear();
		}
		else if (pos.s_pos != last_pos.s_pos)
		{
			list<EventInfo>& leinfo =m_callbackList[m_timePos.s_pos + m_lowCount];
			DealTimeWheeling(leinfo);
			leinfo.clear();
		}
		else if (pos.ms_pos != last_pos.ms_pos)
		{
			list<EventInfo>& leinfo =m_callbackList[m_timePos.ms_pos];
			DealTimeWheeling(leinfo);
			leinfo.clear();
		}
		else
		{
			cout << "error time not change" << endl;
		}
		lock.unlock();
	
}
int TimeWheel::GenerateTimerID()
{
	int x = rand() % 0xffffffff;
	int cur_time = time(nullptr);
	return x | cur_time | m_timerCount;
}

bool TimeWheel::InsertTimer(int diff_ms,EventInfo &einfo)
{
	TimePos time_pos = {0};

	GetNextTrigerPos(diff_ms, time_pos);

	if (time_pos.min_pos != m_timePos.min_pos)
		m_callbackList[m_lowCount + m_midCount + time_pos.min_pos].push_back(einfo);
	else if (time_pos.s_pos != m_timePos.s_pos)
		m_callbackList[m_lowCount + time_pos.s_pos].push_back(einfo);
	else if (time_pos.ms_pos != m_timePos.ms_pos)
		m_callbackList[time_pos.ms_pos].push_back(einfo);

	return true;
}

bool TimeWheel::GetNextTrigerPos(int interval, TimePos& time_pos)
{
	int cur_ms = GetMS(m_timePos);
	int future_ms = cur_ms + interval;

	time_pos.min_pos = (future_ms / 1000 / 60) % m_highCount;
	time_pos.s_pos = (future_ms % (1000 * 60)) / 1000;
	time_pos.ms_pos = (future_ms % 1000) / m_stepMs;

	return true;
}

int TimeWheel::GetMS(TimePos time_pos)
{
	return m_stepMs * time_pos.ms_pos + time_pos.s_pos * 1000 + time_pos.min_pos * 60 * 1000;
}

bool TimeWheel::DealTimeWheeling(std::list<EventInfo> leinfo)
{
	for (auto item = leinfo.begin(); item != leinfo.end(); item++)
	{
		int cur_ms = GetMS(m_timePos);
		int last_ms = GetMS(item->time_pos);
		int diff_ms = (cur_ms - last_ms + (m_highCount + 1) * 60 * 1000) % ((m_highCount + 1) * 60 * 1000);
		if (diff_ms == item->interval)
		{
			item->call_back();

			item->time_pos = m_timePos;
			InsertTimer(item->interval, *item);
		}
		else
		{
			InsertTimer(item->interval - diff_ms, *item);
		}
	}
	return 0;
}
