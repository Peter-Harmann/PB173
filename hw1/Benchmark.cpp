
#include "Benchmark.h"

#include <random>
#include <algorithm>

using namespace std::chrono_literals;
using std::chrono::nanoseconds;

static std::unique_ptr<Sample> bootstrap(const Sample & in, size_t iterations, std::function<std::chrono::nanoseconds(const Sample &)> estimator) {
	std::unique_ptr<Sample> result(new Sample);
	Sample resample;
	size_t size = in.size();
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(0, size - 1);

	for (size_t i = 0; i < iterations; ++i) {
		for (size_t j = 0; j < size; ++j) {
			resample.addSample(in[distribution(generator)]);
		}
		std::chrono::nanoseconds mean = estimator(resample);
		result->addSample(mean);
		resample.clear();
	}
	result->sort();
	return result;
}

static std::chrono::nanoseconds mean(const Sample & sample) {
	return sample.mean();
}


void Sample::addSample(const std::chrono::nanoseconds & time) {
	sorted = false;
	times.push_back(time);
}

void Sample::sort() {
	sorted = true;
	std::sort(times.begin(), times.end());
}

nanoseconds Sample::mean() const {
	nanoseconds sum = 0ns;
	for (nanoseconds i : times) {
		sum += i;
	}
	return sum / times.size();
}

void Benchmark::start() {
	start_time = std::chrono::high_resolution_clock().now();
}

void Benchmark::stop() {
	end_time = std::chrono::high_resolution_clock().now();
}

void Benchmark::run(std::chrono::seconds time) {
	to_run = time;
	bench_start = std::chrono::system_clock().now();

	while (repeat()) {
		func(*this);
		sample.addSample(end_time - start_time);
	}
}

bool Benchmark::repeat() const {
	return to_run > (std::chrono::system_clock().now() - bench_start);
}






