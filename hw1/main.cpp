#include <iostream>
#include "Benchmark.h"

#include <random>
#include <thread>
#include <chrono>

using namespace std;
using namespace std::chrono_literals;


void bench1(Benchmark & bench, unsigned long long iterations) {
	long long a = 65535;
	bench.start();
	for (size_t i = 0; i < iterations; ++i) {
		a = a * a;
	}
	bench.stop();
}

struct RNG {
	std::mt19937 generator;
	RNG() {
		generator.seed(std::random_device()() * static_cast<unsigned int>(std::chrono::high_resolution_clock().now().time_since_epoch().count()));
	}

	operator()(int max) {
		std::uniform_int_distribution<int> distribution(0, max);
		return distribution(generator);
	}
} rng;

void bench2(Benchmark & bench, unsigned long long iterations) {
	int x = rng(100) + iterations;

	//std::cout << "Start 2 Test " << x << std::endl;
	bench.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(x));
	bench.stop();
	//std::cout << "End 2 Test" << std::endl;
}


int main() {
	try {
		std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> start_time, mid_time, end_time;
		
		start_time = std::chrono::high_resolution_clock().now();
		
		BenchmarkSet bench_1("Benchmark 1", bench1);
		BenchmarkSet bench_2("Benchmark 2", bench2);

		bench_1.run(3s, dec_sequence(1000000, 4));
		bench_2.run(3s, sequence(0, 100, 10));
		bench_2.run(3s, sequence(200, 1000, 100));
		
		mid_time = std::chrono::high_resolution_clock().now();
		
		std::cout << bench_1.getStats() << std::endl;
		std::cout << bench_2.getStats() << std::endl;
		
		end_time = std::chrono::high_resolution_clock().now();
		
		std::chrono::milliseconds elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(mid_time - start_time);
		std::cout << "Execution time: " << elapsed_time.count() << "ms." << std::endl;
		elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - mid_time);
		std::cout << "Calculation time: " << elapsed_time.count() << "ms." << std::endl;
		elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
		std::cout << "Total time: " << elapsed_time.count() << "ms." << std::endl;
	}
	catch (const std::exception & e) {
		std::cout << e.what() << std::endl;
	}
    return 0;
}
