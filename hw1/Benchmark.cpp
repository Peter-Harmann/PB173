
#include "Benchmark.h"

#include <random>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <cmath>

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
	nanoseconds sum = std::accumulate(times.begin(), times.end(), 0ns);
	return sum / times.size();
}

nanoseconds Sample::sd(nanoseconds mean) const {
	nanoseconds::rep sum = 0;
	for (nanoseconds i : times) {
		sum += std::pow(i.count() - mean.count(), 2);
	}
	return nanoseconds(static_cast<nanoseconds::rep>(std::sqrt(sum / times.size())));
}

std::chrono::nanoseconds Sample::p95() const {
	if (!sorted) throw std::runtime_error("Sample is not sorted!");

	size_t p95 = (times.size() * 95) / 100;
	return times.at(p95);
}

std::chrono::nanoseconds Sample::p05() const {
	if (!sorted) throw std::runtime_error("Sample is not sorted!");

	size_t p05 = (times.size() * 5 - 99) / 100; // -99 to round down
	return times.at(p05);
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

std::string Benchmark::getStats() const {
	if (!bootstrap_result)				throw std::runtime_error("Results are not bootstraped.");
	if (!bootstrap_result->isSorted())  throw std::runtime_error("Results are not sorted.");

	nanoseconds mean = bootstrap_result->mean();
	nanoseconds sd = bootstrap_result->sd(mean);

	std::string str;
	str = name + ", " + std::to_string((mean - sd).count()) + ", " + std::to_string(bootstrap_result->p05().count()) + ", " + std::to_string(mean.count()) + ", " + std::to_string(bootstrap_result->p95().count()) + ", " + std::to_string((mean + sd).count());
	return str;
}

std::string Benchmark::getStats() {
	if (!bootstrap_result) bootstrap_result.reset(bootstrap(sample, 1000, mean).release());
	if (!bootstrap_result->isSorted()) bootstrap_result->sort();

	return const_cast<const Benchmark *>(this)->getStats();
}





