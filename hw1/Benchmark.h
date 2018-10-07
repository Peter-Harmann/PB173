#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <functional>

class Sample {
	std::vector < std::chrono::nanoseconds> times;

public:
	void addSample(const std::chrono::nanoseconds & time);

	size_t size() const { return times.size(); }
};


class Benchmark {
	std::string name;
	Sample sample;
	std::function<void(Benchmark &)> func;

	std::chrono::seconds to_run;
	std::chrono::time_point<std::chrono::system_clock> bench_start;

	std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> start_time;
	std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> end_time;
public:
	Benchmark(const std::string & name, std::function<void(Benchmark &)> func) : name(name), func(func) {}

	void run(std::chrono::seconds time);

	void start();
	void stop();

	bool repeat() const;

	const Sample & getSample() const { return sample; }
};




/*std::unique_ptr<Sample> bootstrap(const Sample & sample, std::function<double(const Sample &)> estimator, size_t iterations) {
	return nullptr;
}*/


