#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <iostream>

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
	unsigned int iterations = 0;
	std::chrono::time_point<std::chrono::system_clock> bench_start;

	std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> start_time;
	std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> end_time;
public:
	Benchmark(const std::string & name, std::function<void(Benchmark &, unsigned int iterations)> func) : name(name), func(func) {}

	void run(std::chrono::seconds time, unsigned int iterations = 1000000, unsigned char precision = 250);

	void start();
	void stop();

	bool repeat() const;

	std::string getStats();
	std::string getStats() const;

	[[deprecated]] const Sample & getSample() const { return sample; }
};

class Range {
public:
	using op_t = std::function<size_t(size_t)>;
private:
	size_t actual;
	size_t last;
	op_t op;
public:
	Range() : actual(1), last(0) {}
	Range(size_t actual, size_t last, op_t op = [](size_t a) { return a + 1; }) : actual(actual), last(last), op(op) {}
	Range(const Range & o) : actual(o.actual), last(o.last), op(o.op) {}
	Range(Range && o) : actual(o.actual), last(o.last), op(std::move(o.op)) {}

	Range & operator=(const Range & o) { 
		actual = o.actual;
		last = o.last;
		op = o.op;
		return *this;
	}
	Range & operator=(Range && o) {
		actual = o.actual;
		last = o.last;
		op = std::move(o.op);
		return *this;
	}

	Range & operator++() {
		actual = op(actual);
		return *this;
	}

	bool operator==(const Range & o) {
		if (this->actual > this->last && o.actual > o.last) return true;
		return o.actual == this->actual;
	}
	
	bool operator!=(const Range & o) {
		if (this->actual > this->last && o.actual > o.last) return false;
		return o.actual != this->actual;
	}
	
	size_t operator*() {
		return actual;
	}
	
	bool valid() const {
		return (actual <= last);
	}
};

class BenchmarkSet {
	std::string name;
	std::function<void(Benchmark &, unsigned int iterations)> func;

	std::map<size_t, std::unique_ptr<Benchmark>> benchmarks;
public:
	BenchmarkSet(const std::string & name, std::function<void(Benchmark &, unsigned int iterations)> func) : name(name), func(func) {}

	void run(std::chrono::seconds time, unsigned int iterations = 1000000, unsigned char precision = 250);
	void run(std::chrono::seconds time, Range first, unsigned char precision = 250) {
		for (; first.valid(); ++first) {
			this->run(time, *first, precision);
		}
	}
	
	template <class RandomIt>
	void run(std::chrono::seconds time, RandomIt first, RandomIt last, unsigned char precision = 250) {
		for (auto i = first; i != last; ++i) {
			this->run(time, *i, precision);
		}
	}

	std::string getStats();
	std::string getStats() const;
};

inline Range sequence(size_t start, size_t end, size_t step = 1) { return Range(start, end, std::bind([](size_t a, size_t b) { return a+b; }, std::placeholders::_1, step)); }
inline Range exe_sequence(size_t start, size_t end, size_t step = 10) { return Range(start, end, std::bind([](size_t a, size_t b) { return a*b; }, std::placeholders::_1, step)); }
inline Range bin_sequence(size_t start, size_t num) { 
	size_t end = start;
	for(size_t i=1; i< num; ++i) end *= 2;  
	return Range(start, end, [](size_t a) { return a*2; }); 
}
inline Range dec_sequence(size_t start, size_t num) { 
	size_t end = start;
	for(size_t i=1; i< num; ++i) end *= 10;  
	return Range(start, end, [](size_t a) { return a*10; }); 
}
