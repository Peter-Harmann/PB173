#include <iostream>
#include "Benchmark.h"

#include <random>
#include <thread>
#include <chrono>
#include <stdexcept>

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


int main(int argc, char ** argv) {
	try {
		std::chrono::seconds time = 3s;
		unsigned char precision = 250;
		if(argc == 3) {
			if(std::string(argv[1]) == "time") {
				time = std::chrono::seconds(std::atoi(argv[2]));
				if(time < 1s || time > 300s) {
					throw std::runtime_error("Precision out of range!");
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
		
		BenchmarkSet bench_1("Benchmark 1", bench1, 60s);
		BenchmarkSet bench_2("Benchmark 2", bench2, 10s);
		
		start_time = std::chrono::high_resolution_clock().now();

		bench_1.run(time, dec_sequence(1000000, 4), precision);
		bench_2.run(time, sequence(0, 100, 10), precision);
		bench_2.run(time, sequence(200, 1000, 100), precision);
		
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
