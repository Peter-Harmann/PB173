
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>

#include "../hw1/Benchmark.h"

using namespace std::literals::chrono_literals;



template <class DataType>
class MapReduce {
	unsigned int num_threads = 4;
	std::vector<std::unique_ptr<std::thread>> threads;
public:	
	template<class DataFunc>
	DataType execute(const std::vector<DataType> & data, DataFunc func);
	
	void set_threads(unsigned int threads) { num_threads = threads; } 
};

template <class DataType> template <class DataFunc>
DataType MapReduce<DataType>::execute(const std::vector<DataType> & data, DataFunc func) {
	size_t size = data.size();
	const DataType * data_ptr = data.data();
	size_t block_size = size / num_threads;
	
	for(unsigned int i = 1; i < num_threads; ++i) {
		threads.push_back(std::make_unique<std::thread>(func, data_ptr, data_ptr + block_size));
		data_ptr += block_size;
	}
	threads.push_back(std::make_unique<std::thread>(func, data_ptr, data_ptr + size - (num_threads - 1)*block_size));
	
	for(auto & i : threads) {
		i->join();
	}
	return 0;
}

std::atomic<long long> result(0);
void sum(const long long * data, const long long * data_end) {
	long long res = 0;
	while(data != data_end) {
		res += *data;
		++data;
	}
	result += res;
}

const int data_size = 10000000;
void bench1(Benchmark & bench, unsigned long long iterations) {
	MapReduce<long long> map_reduce;
	map_reduce.set_threads(iterations);
	
	std::vector<long long> data;
	data.resize(data_size);
	for(auto & i : data) {
		i = 100;
	}
	
	bench.start();
	map_reduce.execute(data, sum);
	bench.stop();
}	

int main(int argc, char ** argv) {
	std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> start_time, mid_time, end_time;
		
	BenchmarkSet bench_1("MapReduce - Sum", 	bench1, 300s);
	
	start_time = std::chrono::high_resolution_clock().now();
		
	// Basic sizes
	bench_1.run(5s, 1, 250);
	bench_1.run(5s, 2, 250);
	bench_1.run(5s, 4, 250);
	bench_1.run(5s, 8, 250);
	
	mid_time = std::chrono::high_resolution_clock().now();
		
	std::cout << bench_1.getStats() << std::flush;
	
	std::cout << std::endl;
		
	end_time = std::chrono::high_resolution_clock().now();
		
	// Uncomment for statistics about total time of all benchmarks
		
	std::chrono::milliseconds elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(mid_time - start_time);
	std::cout << "Execution time: " << elapsed_time.count() << "ms" << std::endl;
	elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - mid_time);
	std::cout << "Calculation time: " << elapsed_time.count() << "ms" << std::endl;
	elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	std::cout << "Total time: " << elapsed_time.count() << "ms" << std::endl;
	
	std::cout << result.load() / 100 / data_size << std::endl;
	return 1;
}









