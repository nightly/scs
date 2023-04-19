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
		output_stream_ << "Timer for " << name_ << " took " << duration << " microseconds or " << (duration * 0.001) << " ms (milliseconds) \n";
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