#include "GameTimer.h"

GameTimer::GameTimer()
	: m_secondsPerCount(0.0)
	, m_deltaTime(-1.0)
	, m_baseTime(0)
	, m_pausedTime(0)
	, m_prevTime(0)
	, m_currTime(0)
	, m_stopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_secondsPerCount = 1.0 / (double)countsPerSec;
}


GameTimer::~GameTimer()
{

}

float GameTimer::TotalTime() const
{
	if (m_stopped)
	{
		return static_cast<float>((m_stopTime - m_baseTime - m_pausedTime)*m_secondsPerCount);
	}
	else
	{
		return static_cast<float>((m_currTime - m_baseTime - m_pausedTime)*m_secondsPerCount);
	}
}

float GameTimer::DeltaTime() const
{
	return static_cast<float>(m_deltaTime);
}

void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_baseTime = currTime;
	m_prevTime = currTime;
	m_stopTime = 0;
	m_stopped = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (m_stopped)
	{
		//更新暂停的总时间
		m_pausedTime += (startTime - m_stopTime);
		m_prevTime = startTime;
		//不再暂停
		m_stopTime = 0;
		m_stopped = false;
	}

}

void GameTimer::Stop()
{
	if (!m_stopped)
	{
		__int64 currentTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

		m_stopTime = currentTime;
		m_stopped = true;
	}

}

void GameTimer::Tick()
{
	if (m_stopped)
	{
		m_deltaTime = 0.0;
		return;
	}
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_currTime = currTime;
	// 当前帧和上一帧之间的时间差
	m_deltaTime = (m_currTime - m_prevTime)* m_secondsPerCount;
	// 为计算下一帧做准备
	m_prevTime = m_currTime;

	// 确保不为负值。DXSDK中的CDXUTTimer提到：如果处理器进入了节电模式
	// 或切换到另一个处理器，m_deltaTime会变为负值。
	if (m_deltaTime < 0.0)
	{
		m_deltaTime = 0.0;
	}

}

