#include "Timer.h"

//Define the timer class without a name
CTimer::CTimer() {
	m_name = "";
	m_start = clock();
	m_end = 0;
	//cout << "Start Function <" << m_name << ">..." << endl;
}

//Define the timer class with a specific name 
CTimer::CTimer(string timerName, bool isTiming) {
	assert(timerName.length() > 0);
	m_isTime = isTiming; 
	m_name = timerName;
	m_start = clock();
	//cout << m_start << endl; 
	/*if (isTiming)
		cout << "Start Function <" << m_name << ">..." << endl;*/
}

//End the timing when destroyed 
CTimer::~CTimer() {
	m_end = clock();
	if (m_isTime)
		cout << "Timing <" << m_name << "> " << (m_end - m_start) / (double) CLOCKS_PER_SEC << endl;
}

double CTimer::Time() {
	m_end = clock();
	
	double t = (m_end - m_start) / (double) CLOCKS_PER_SEC;
	return t; 
}
