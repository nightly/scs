#include "scs/common/timer.h"

#include <chrono>
#include <string>
#include <iostream>

namespace scs {

	Timer::Timer(const std::string& name, std::ostream& output_stream)
		: name_(name), output_stream_(output_stream), stopped_(false) {
		start_ = std::chrono::high_resolution_clock::now();
	}

	long long Timer::Stop() {
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(start_).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
		stopped_ = true;
		return end - start;
	}

	void Timer::StopWithWrite() {
		auto duration = Stop();
		WriteDuration(duration);
		stopped_ = true;
	}

	void Timer::Restart() {
		stopped_ = false;
		start_ = std::chrono::high_resolution_clock::now();
	}

	void Timer::WriteDuration(long long duration) {
		long long milliseconds = duration / 1000;
		long long total_seconds = milliseconds / 1000;
		long long minutes = total_seconds / 60;
		long long seconds = total_seconds % 60;

		output_stream_ << "Timer for " << name_ << " took " << duration << " microseconds, "
			<< milliseconds << " ms, "
			<< minutes << ":" << seconds << " (mm:ss) \n";
		output_stream_.flush();
	}

	Timer::~Timer() {
		if (!stopped_) {
			StopWithWrite();
		}
		else {
			Stop();
		}
	}

}