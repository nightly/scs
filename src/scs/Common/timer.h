#pragma once 

#include <chrono>
#include <string>
#include <iostream>

namespace scs {

	// RAII Timer
	class Timer {
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> start_;
		std::string name_;
		std::ostream& output_stream_;
		bool stopped_;
	public:
		Timer(const std::string& name, std::ostream& output_stream = std::cout);
		~Timer();
		void Restart();
		long long Stop();
		void WriteDuration(long long duration);
		void StopWithWrite();
	};
}