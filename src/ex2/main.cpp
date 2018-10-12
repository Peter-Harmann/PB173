
#include <iostream>
#include "../hw1/Benchmark.h"

#include <random>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <vector>
#include <list>

using namespace std;
using namespace std::chrono_literals;

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

void bench1(Benchmark & bench, unsigned long long iterations) {
	std::vector<int> vec;
	vec.resize(iterations*10);
	int sum = 0;

	bench.start();
	for (size_t j = 0; j < 10000; ++j) {
		size_t rnd = rng(iterations*9);
		for (size_t i = 0; i < iterations; ++i) {
			sum += vec[i + rnd];
		}
	}
	bench.stop();
}

void bench2(Benchmark & bench, unsigned long long iterations) {
	std::list<int> list;
	list.resize(iterations);
	int sum = 0;

	bench.start();
	for (size_t j = 0; j < 10000; ++j) {
		for (int i : list) {
			sum += i;
		}
	}
	bench.stop();
}


int main(int argc, char ** argv) {
	try {
		std::chrono::seconds time = 3s;
		unsigned char precision = 250;
		if (argc == 3) {
			if (std::string(argv[1]) == "time") {
				time = std::chrono::seconds(std::atoi(argv[2]));
				if (time < 1s || time > 6000s) {
					throw std::runtime_error("Precision out of range!");
				}
			}
			else if (std::string(argv[1]) == "prec") {
				int prec = std::atoi(argv[2]);
				if (prec < 1 || prec > 200) {
					throw std::runtime_error("Precision out of range!");
				}
				precision = prec;
			}
			else throw std::runtime_error("Bad argument 1!");
		}
		else if (argc != 1) throw std::runtime_error("Bad argument count!");

		/*
		std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> test_test;
		test_test = std::chrono::high_resolution_clock().now();
		while (std::chrono::high_resolution_clock().now() == test_test) {}
		std::cout << "Clock time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock().now() - test_test).count() << "ms" << std::endl;
		*/

		std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> start_time, mid_time, end_time;

		BenchmarkSet bench_1("Vector", bench1, 60s);
		BenchmarkSet bench_2("List", bench2, 60s);

		start_time = std::chrono::high_resolution_clock().now();

		bench_1.run(time, dec_sequence(10, 5), precision);
		bench_2.run(time, dec_sequence(10, 5), precision);

		mid_time = std::chrono::high_resolution_clock().now();

		std::cout << bench_1.getStats() << std::endl;
		std::cout << bench_2.getStats() << std::endl;

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


































