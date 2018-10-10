#include <iostream>
#include "Benchmark.h"

#include <random>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <vector>
#include <algorithm>

using namespace std;
using namespace std::chrono_literals;

struct RNG {
	std::mt19937 generator;
	RNG() {
		generator.seed(std::random_device()() * static_cast<unsigned int>(std::chrono::high_resolution_clock().now().time_since_epoch().count()));
	}

	int operator()(int max) {
		std::uniform_int_distribution<int> distribution(0, max);
		return distribution(generator);
	}

	double operator()(double max) {
		std::uniform_real_distribution <double> distribution(0.0, max);
		return distribution(generator);
	}

	std::function<int()> getDistribution(int min, int max) {
		std::uniform_int_distribution<int> distribution(min, max);
		return std::bind(distribution, generator);
	}

	std::function<double()> getDistribution(double min, double max) {
		std::uniform_real_distribution<double> distribution(min, max);
		return std::bind(distribution, generator);
	}
} rng;

void bench1(Benchmark & bench, unsigned long long iterations) {
	if (iterations >= 10000) {
		std::vector<double> vec;
		vec.resize(iterations);
		auto d = rng.getDistribution(0.0, 1000000.0);
		for (size_t i = 0; i < iterations; ++i) {
			vec[i] = d();
		}

		bench.start();
		std::sort(vec.begin(), vec.end());
		bench.stop();
	}
	else {
		size_t repeats = (20000 / iterations); // Get a reasonable number of repeats 
		size_t size = repeats * iterations;
		std::vector<double> vec;
		vec.resize(size);
		auto d = rng.getDistribution(0.0, 1000000.0);
		for (size_t i = 0; i < size; ++i) {
			vec[i] = d();
		}

		bench.start();
		for (size_t i = 0; i < repeats; ++i) {
			std::sort(vec.begin() + i * iterations, vec.begin() + i*iterations + iterations);
		}
		bench.stop();
		bench.set_repeats(repeats);
	}
}

void bench2(Benchmark & bench, unsigned long long iterations) {
	int x = rng(100) + iterations;

	//std::cout << "Start 2 Test " << x << std::endl;
	bench.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(x));
	bench.stop();
	//std::cout << "End 2 Test" << std::endl;
}


int main(int argc, char ** argv) {
	try {
		std::chrono::seconds time = 3s;
		unsigned char precision = 250;
		if(argc == 3) {
			if(std::string(argv[1]) == "time") {
				time = std::chrono::seconds(std::atoi(argv[2]));
				if(time < 1s || time > 604800s) {
					throw std::runtime_error("Time out of range!");
				}
			}
			else if(std::string(argv[1]) == "prec") {
				int prec = std::atoi(argv[2]);
				if(prec < 1 || prec > 200) {
					throw std::runtime_error("Precision out of range!");
				}
				precision = prec;
			}
			else throw std::runtime_error("Bad argument 1!");
		}
		else if(argc != 1) throw std::runtime_error("Bad argument count!");
		
		std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> start_time, mid_time, end_time;
		
		BenchmarkSet bench_1("std::sort", bench1, 120s);
		
		start_time = std::chrono::high_resolution_clock().now();

		// Precision for very small sizes gets bad
		//bench_1.run(time, dec_sequence(10, 5), precision);
		//bench_1.run(time, bin_sequence(16, 15), precision);

		// A more reasonable sizes
		bench_1.run(time, dec_sequence(100, 4), precision);
		bench_1.run(time, bin_sequence(128, 12), precision);

		// Use for a single test run
		//bench_1.run(time, 262144, precision);
		
		mid_time = std::chrono::high_resolution_clock().now();
		
		std::cout << bench_1.getStats() << std::endl;
		
		end_time = std::chrono::high_resolution_clock().now();
		
		std::chrono::milliseconds elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(mid_time - start_time);
		std::cout << "Execution time: " << elapsed_time.count() << "ms" << std::endl;
		elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - mid_time);
		std::cout << "Calculation time: " << elapsed_time.count() << "ms" << std::endl;
		elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
		std::cout << "Total time: " << elapsed_time.count() << "ms" << std::endl;
	}
	catch (const std::exception & e) {
		std::cout << e.what() << std::endl;
	}
    return 0;
}
