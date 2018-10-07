#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <map>
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
	bool isSorted() const { return sorted; }

	std::chrono::nanoseconds mean() const;
	std::chrono::nanoseconds sd(std::chrono::nanoseconds mean) const;
	std::chrono::nanoseconds sd() const { return this->sd(this->mean()); }
	std::chrono::nanoseconds var(std::chrono::nanoseconds mean) const;
	std::chrono::nanoseconds p95() const;
	std::chrono::nanoseconds p05() const;

	[[deprecated]] void print() const;
};

class Benchmark {
	std::string name;
	Sample sample;
	std::unique_ptr<Sample> bootstrap_result;

	std::function<void(Benchmark &, unsigned int iterations)> func;

	std::chrono::seconds to_run;
	unsigned char precision = 100;
	std::chrono::time_point<std::chrono::system_clock> bench_start;

	std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> start_time;
	std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> end_time;
public:
	Benchmark(const std::string & name, std::function<void(Benchmark &, unsigned int iterations)> func) : name(name), func(func) {}

	void run(std::chrono::seconds time, unsigned int iterations = 1000000, unsigned char precision = 100);

	void start();
	void stop();

	bool repeat() const;

	std::string getStats();
	std::string getStats() const;

	[[deprecated]] const Sample & getSample() const { return sample; }
};

class BenchmarkSet {
	std::string name;
	std::function<void(Benchmark &, unsigned int iterations)> func;

	std::map<size_t, std::unique_ptr<Benchmark>> benchmarks;
public:
	BenchmarkSet(const std::string & name, std::function<void(Benchmark &, unsigned int iterations)> func) : name(name), func(func) {}

	void run(std::chrono::seconds time, unsigned int iterations = 1000000);
};


