#include "Timer.h"
#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <string>

using namespace std::chrono;

static high_resolution_clock::time_point t1;
static high_resolution_clock::time_point t2;

Timer::Timer()
{
}


Timer::~Timer()
{
}

std::string getReadableString(double duration) {
	duration = duration * 1000;

	int hours = duration / 3600000;
	duration = duration - 3600000 * hours;
	
	int minutes = duration / 60000;
	duration = duration - 60000 * minutes;

	int seconds = duration / 1000;
	int milliseconds = duration - 1000 * seconds;

	std::string hr = std::to_string(hours);
	std::string min = std::to_string(minutes);
	std::string sec = std::to_string(seconds);
	std::string mil = std::to_string(milliseconds);

	if (hours < 10) {
		hr = "0" + hr;
	}

	if (minutes < 10) {
		min = "0" + min;
	}

	if (seconds < 10) {
		sec = "0" + sec;
	}

	if (milliseconds < 100) {
		mil = "0" + mil;
	}

	if (milliseconds < 10) {
		mil = "0" + mil;
	}

	return hr + ":" + min + ":" + sec + "." + mil;
}

void Timer::start()
{
	t1 = high_resolution_clock::now();
}

void Timer::stop()
{
	t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

	std::cout << getReadableString(time_span.count()) << ";";
}