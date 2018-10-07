
#include "Benchmark.h"

#include <random>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <cmath>
#include <iostream>

const size_t bootstrap_n = 1000;

using namespace std::chrono_literals;
using std::chrono::nanoseconds;

static struct RNG {
	std::default_random_engine generator;
	unsigned int seed = 1;

	RNG() {
		seed *= std::random_device()();
		seed *= static_cast<unsigned int>(std::chrono::high_resolution_clock().now().time_since_epoch().count());
		generator.seed(seed);
	}

	void addEntrppy() {
		seed *= std::random_device()();
		seed *= static_cast<unsigned int>(std::chrono::high_resolution_clock().now().time_since_epoch().count());
		generator.seed(seed);
	}

	void addEntropy(unsigned int entropy) {
		seed ^= entropy;
		generator.seed(seed);
	}

	int operator()(int min, int max) {
		std::uniform_int_distribution<int> distribution(min, max);
		return distribution(generator);
	}

	std::function<int()> getDistribution(int min, int max) {
		std::uniform_int_distribution<int> distribution(min, max);
		return std::bind(distribution, generator);
	}
} rng;


static std::unique_ptr<Sample> bootstrap(const Sample & in, size_t iterations, std::function<std::chrono::nanoseconds(Sample &)> estimator) {
	std::unique_ptr<Sample> result(new Sample);
	Sample resample;
	size_t size = in.size();

	auto random_num = rng.getDistribution(0, size - 1);

	for (size_t i = 0; i < iterations; ++i) {
		for (size_t j = 0; j < size; ++j) {
			resample.addSample(in[random_num()]);
		}
		nanoseconds mean = estimator(resample);
		result->addSample(mean);
		resample.clear();
	}
	result->sort();
	return result;
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
		sum += (i.count() - mean.count()) * (i.count() - mean.count());
	}
	return nanoseconds(static_cast<nanoseconds::rep>(std::sqrt(sum / times.size())));
}

nanoseconds Sample::var(nanoseconds mean) const {
	nanoseconds::rep sum = 0;
	for (nanoseconds i : times) {
		sum += (i.count() - mean.count()) * (i.count() - mean.count());
	}
	return nanoseconds(sum / times.size());
}

std::chrono::nanoseconds Sample::p95() {
	if (!sorted) sort();
	
	size_t p95 = (times.size() * 95) / 100;
	return times.at(p95);
}

std::chrono::nanoseconds Sample::p05() {
	if (!sorted) sort();

	size_t p05 = (times.size() * 5) / 100;
	return times.at(p05);
}

/*
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
*/

void Sample::print() const {
	for (nanoseconds i : times) {
		std::cout << i.count() << ", " << std::endl;
	}
}

void Benchmark::start() {
	start_time = std::chrono::high_resolution_clock().now();
}

void Benchmark::stop() {
	end_time = std::chrono::high_resolution_clock().now();
}

void Benchmark::run(std::chrono::seconds time, unsigned long long iterations, unsigned char precision) {
	bootstrap_result.reset();
	to_run = time;
	bench_start = std::chrono::system_clock().now();
	this->precision = precision;
	this->iterations = iterations;

	while (repeat()) {
		func(*this, iterations);
		sample.addSample(end_time - start_time);
	}
}

bool Benchmark::repeat() {
	if(to_run > (std::chrono::system_clock().now() - bench_start)) return true;
	if (precision == 250) return false;

	std::unique_ptr<Sample> bootstrap_res = bootstrap(sample, bootstrap_n, Sample::mean);
	nanoseconds sample_mean = bootstrap_res->mean();
	nanoseconds sample_sd = bootstrap_res->sd(sample_mean);

	if (4 * sample_sd * 100 > precision * sample_mean) return true;
	
	// Why? ;(
	auto bootstrap_p05(bootstrap(sample, bootstrap_n, Sample::p05));
	auto bootstrap_p95(bootstrap(sample, bootstrap_n, Sample::p95));
	nanoseconds p05 = bootstrap_p05->mean();
	nanoseconds p95 = bootstrap_p95->mean();
	
	if ((p95 - p05) * 100 > precision * sample_mean) return true;
	
	bootstrap_result.reset(bootstrap_res.release());
	return false;
}

/*std::string Benchmark::getStats() const {
	if (!bootstrap_result)				throw std::runtime_error("Results are not bootstraped.");
	if (!bootstrap_result->isSorted())  throw std::runtime_error("Results are not sorted.");
	
	return str;
}*/

std::string Benchmark::getStats() {
	if (!bootstrap_result) bootstrap_result.reset(bootstrap(sample, bootstrap_n, Sample::mean).release());
	if (!bootstrap_result->isSorted()) bootstrap_result->sort();
	
	auto bootstrap_p05(bootstrap(sample, bootstrap_n, Sample::p05));
	auto bootstrap_p95(bootstrap(sample, bootstrap_n, Sample::p95));

	nanoseconds mean = bootstrap_result->mean();
	nanoseconds sd = bootstrap_result->sd(mean);

	nanoseconds p05 = bootstrap_p05->mean();
	nanoseconds p95 = bootstrap_p95->mean();
	
	std::string str;
	str = name + ", " + std::to_string(iterations) + ", " + std::to_string((mean - 2 * sd).count()) + ", " + std::to_string(p05.count()) + ", " + std::to_string(mean.count()) + ", " + std::to_string(p95.count()) + ", " + std::to_string((mean + 2 * sd).count());
	return str;
}




void BenchmarkSet::run(std::chrono::seconds time, unsigned long long iterations, unsigned char precision) {
	auto it = benchmarks.emplace(iterations, std::make_unique<Benchmark>(name, func));

	if (!it.second) throw std::runtime_error("Already run for this amount of iterations! " + std::to_string(iterations));

	Benchmark & bench = *it.first->second;
	bench.run(time, iterations, precision);
}

std::string BenchmarkSet::getStats() {
	std::string str;
	for (auto & i : benchmarks) {
		str += i.second->getStats() + "\n";
	}
	return str;
}

std::string BenchmarkSet::getStats() const {
	std::string str;
	for (auto & i : benchmarks) {
		str += i.second->getStats() + "\n";
	}
	return str;
}
