#pragma once
#include <Windows.h>

class GameTimer
{
public:
	GameTimer();
	~GameTimer();
	float TotalTime()const; // ��λΪ��
	float DeltaTime()const; // ��λΪ��
	void Reset();   // ��Ϣѭ��ǰ����
	void Start();   // ȡ����ͣʱ����
	void Stop();    // ��ͣʱ����
	void Tick();    // ÿ֡����
private:
	double m_secondsPerCount;
	double m_deltaTime;

	__int64 m_baseTime;
	__int64 m_pausedTime;
	__int64 m_stopTime;
	__int64 m_prevTime;
	__int64 m_currTime;

	bool m_stopped;
};

