#include <iostream>
#include "Benchmark.h"

#include <random>
#include <thread>
#include <chrono>

using namespace std;
using namespace std::chrono_literals;


void bench1(Benchmark & bench, unsigned int iterations) {
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

void bench2(Benchmark & bench, unsigned int iterations) {
	int x = rng(100) + iterations;

	//std::cout << "Start 2 Test " << x << std::endl;
	bench.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(x));
	bench.stop();
	//std::cout << "End 2 Test" << std::endl;
}


int main() {
	try {
		BenchmarkSet bench_1("Benchmark 1", bench1);
		Benchmark bench_2("Benchmark 2", bench2);

		std::vector<size_t> vec { 10000, 100000, 1000000 };

		std::cout << "Start 1" << std::endl;
		bench_1.run(3s, Range(10000, 1000000, 10, [](size_t a, size_t b) { return a * b; }), Range());
		std::cout << "Start 2" << std::endl;
		//bench_2.run(3s, 50, 20);

		std::cout << bench_1.getStats() << std::endl;
		//std::cout << bench_2.getStats() << std::endl;
	}
	catch (const std::exception & e) {
		std::cout << e.what() << std::endl;
	}
    return 0;
}
