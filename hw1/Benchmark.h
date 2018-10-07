#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <functional>

class Sample {
	bool sorted = false;
	std::vector < std::chrono::nanoseconds> times;

public:
	Sample() {}
	Sample(const Sample & o) : times(o.times) {}
	Sample(Sample && o) : times(std::move(o.times)) {}

	void addSample(const std::chrono::nanoseconds & time);

	std::chrono::nanoseconds operator[](size_t index) const { return times[index]; }
	size_t size() const { return times.size(); }
	void clear() { times.clear(); }
	void sort();

	std::chrono::nanoseconds mean() const;
};

class Benchmark {
	std::string name;
	Sample sample;
	std::unique_ptr<Sample> bootstrap_result;

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

	[[deprecated]] const Sample & getSample() const { return sample; }
};


