#include <iostream>
#include "../hw1/Benchmark.h"

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

using Matrix = std::vector<std::vector<double>>;

static Matrix getRandomMatrix(size_t width, size_t height) {
	auto dis = rng.getDistribution(-10000.0, 10000.0);
	
	Matrix matrix;
	matrix.resize(height);
	for(size_t i=0; i<height; ++i) {
		matrix[i].resize(width);
		for(size_t j=0; j<width; ++j) {
			matrix[i][j] = dis();
		}
	}
	return matrix;
} 

void bench1(Benchmark & bench, unsigned long long iterations) {
	Matrix m1 = getRandomMatrix(iterations, iterations);
	Matrix m2 = getRandomMatrix(iterations, iterations);
	Matrix m3;
	m3.resize(iterations);
	for(size_t i=0; i<iterations; ++i) {
		m3[i].resize(iterations);
	}
	
	bench.start();
	for(size_t i=0; i<iterations; ++i) {
		for(size_t j=0; j<iterations; ++j) {
			double sum = 0;
			for(size_t k=0; k<iterations; ++k) {
				sum += m1[i][k] * m2[k][j];
			}
			m3[i][j] = sum;
		}
	}
	bench.stop();
}

void bench2(Benchmark & bench, unsigned long long iterations) {
	Matrix m1 = getRandomMatrix(iterations, iterations);
	Matrix m2 = getRandomMatrix(iterations, iterations);
	Matrix m3;
	m3.resize(iterations);
	for(size_t i=0; i<iterations; ++i) {
		m3[i].resize(iterations);
	}
	
	bench.start();
	for (size_t i = 0; i < iterations; ++i) { 
		double t = m1[i][0];
		for (size_t j = 0; j < iterations; ++j)
			m3[i][j] = t * m2[0][j];

		for (size_t k = 1; k < iterations; ++k) {
			t = m1[i][k];
			for (size_t j = 0; j < iterations; ++j)
				m3[i][j] += t * m2[k][j];
		}
	}
	bench.stop();
}

void bench3(Benchmark & bench, unsigned long long iterations) {
	Matrix m1 = getRandomMatrix(iterations, iterations);
	Matrix m2 = getRandomMatrix(iterations, iterations);
	Matrix m3;
	m3.resize(iterations);
	for(size_t i=0; i<iterations; ++i) {
		m3[i].resize(iterations);
	}
	
	bench.start();
	std::vector<double> col;
	col.resize(iterations);
	for(size_t j=0; j<iterations; ++j) {
		for (size_t k = 0; k < iterations; ++k)
			col[k] = m2[k][j];

		for (size_t i = 0; i < iterations; ++i) { 
			double sum = 0;
			for(size_t k=0; k<iterations; ++k) {
				sum += m1[i][k] * col[k];
			}
			m3[i][j] = sum;
		}
	}
	bench.stop();
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
		
		BenchmarkSet bench_1("Natural", 	bench1, 120s);
		BenchmarkSet bench_2("Reordered", 	bench2, 120s);
		BenchmarkSet bench_3("Col. Copy", 	bench3, 120s);
		
		start_time = std::chrono::high_resolution_clock().now();

		// A more reasonable sizes
		bench_1.run(time, bin_sequence(16, 6), precision);
		bench_2.run(time, bin_sequence(16, 6), precision);
		bench_3.run(time, bin_sequence(16, 6), precision);
		
		mid_time = std::chrono::high_resolution_clock().now();
		
		std::cout << bench_1.getStats() << std::endl;
		std::cout << bench_2.getStats() << std::endl;
		std::cout << bench_3.getStats() << std::endl;
		
		end_time = std::chrono::high_resolution_clock().now();
		
		// Uncomment for statistics about total time of all benchmarks
		/*
		std::chrono::milliseconds elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(mid_time - start_time);
		std::cout << "Execution time: " << elapsed_time.count() << "ms" << std::endl;
		elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - mid_time);
		std::cout << "Calculation time: " << elapsed_time.count() << "ms" << std::endl;
		elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
		std::cout << "Total time: " << elapsed_time.count() << "ms" << std::endl;
		*/
	}
	catch (const std::exception & e) {
		std::cout << e.what() << std::endl;
	}
    return 0;
}
