#include <iostream>
#include "../hw1/Benchmark.h"

#include <random>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <bitset>
#include <array>

using namespace std;
using namespace std::literals::chrono_literals;

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

class Bitvector {
	std::bitset<65536> number;
	
public:
	void insert(uint16_t num) { number.set(num, true); }
	void erase(uint16_t num){ number.set(num, false); }
	bool test(uint16_t num) const { return number.test(num); }
	bool empty() const { return number.any(); }
	
	Bitvector* union_op(class Bitvector & o) {
		Bitvector* res = new Bitvector;
		res->number = this->number | o.number;
		return res;
	}
	Bitvector* intersection_op(class Bitvector & o){
		Bitvector* res = new Bitvector;
		res->number = this->number & o.number;
		return res;
	}
};

class Trie {
	static Trie * const LeafTrie;
	std::array<Trie*, 16> other;
public:
	bool empty() const {
		for(size_t i = 0; i < 16; ++i) {
			if(other[i] != nullptr) return false;
		}
		return true;
	}

	Trie() {
		for(size_t i = 0; i<16; ++i) other[i] = nullptr;
	}
	Trie(const Trie & o) {
		for(size_t i=0; i<16; ++i) {
			if(o.other[i]) {
				if (o.other[i] == LeafTrie) this->other[i] = LeafTrie;
				else this->other[i] = new Trie(*o.other[i]);
			}
			else this->other[i] = nullptr;
		}
	}
	Trie(Trie && o) {
		for (size_t i = 0; i < 16; ++i) {
			other[i] = o.other[i];
			o.other[i] = nullptr;
		}
	}
	~Trie() {
		for (size_t i = 0; i < 16; ++i) {
			if (other[i] != LeafTrie) {
				delete other[i];
			}
			other[i] = nullptr;
		}
	}
	Trie & operator=(const Trie &) = delete;
	Trie & operator=(Trie &&) = delete;

	void insert(uint16_t num, unsigned int level = 4) {
		Trie* & o = other[num & 0xF];
		if(level == 1) o = LeafTrie;
		else if(o == nullptr) {
			Trie * nt = new Trie;
			nt->insert(num >> 4, level-1);
			o = nt;
		}
		else o->insert(num >> 4, level-1);
	}
	void erase(uint16_t num, unsigned int level = 4) { 
		Trie* & o = other[num & 0xF];
		if(level == 1) o = nullptr;
		else if(o != nullptr) {
			o->erase(num >> 4, level-1);
			if(o->empty()) {
				delete o;
				o = nullptr;
			}
		}	
	}
	
	Trie* union_op(const Trie & o, unsigned int level = 4) {
		Trie* res = new Trie;
		if(level == 1) {
			for(size_t i=0; i<16; ++i) {
				if(o.other[i] || this->other[i]) res->other[i] = LeafTrie;
			}
		}
		else {
			for(size_t i=0; i<16; ++i) {
				if(this->other[i]) {
					if(o.other[i]) res->other[i] = this->other[i]->union_op(*o.other[i], level-1);
					else res->other[i] = new Trie(*this->other[i]);
				}
				else if(o.other[i]) res->other[i] = new Trie(*o.other[i]);
			}
		}
		return res;
	}
	
	Trie* intersection_op(const Trie & o, unsigned int level = 4){
		Trie* res = new Trie;
		if(level == 1) {
			for(size_t i=0; i<16; ++i) {
				if(o.other[i] && this->other[i]) res->other[i] = LeafTrie;
			}
		}
		else {
			for(size_t i=0; i<16; ++i) {
				if(this->other[i] && o.other[i]) res->other[i] = this->other[i]->intersection_op(*o.other[i],level-1);
			}
		}
		if(res->empty() && level < 4) {
			delete res;
			return nullptr;
		}
		return res;
	}
};

Trie* const Trie::LeafTrie = (Trie *)0xFFFFFFFF;

void bench1(Benchmark & bench, unsigned long long iterations) {
	Bitvector test;
	auto dis = rng.getDistribution(0, 65535);
	
	bench.start();
	for(size_t i=0; i<iterations; ++i) {
		test.insert(dis());
	}
	bench.stop();
}

void bench2(Benchmark & bench, unsigned long long iterations) {
	Trie test;
	auto dis = rng.getDistribution(0, 65535);
	
	bench.start();
	for(size_t i=0; i<iterations; ++i) {
		test.insert(dis());
	}
	bench.stop();
}

void bench3(Benchmark & bench, unsigned long long iterations) {
	Bitvector test;
	auto dis = rng.getDistribution(0, 65535);
	
	for(size_t i=0; i<40000; ++i) {
		test.insert(dis());
	}
	
	bench.start();
	for(size_t i=0; i<iterations; ++i) {
		test.erase(dis());
	}
	bench.stop();
}

void bench4(Benchmark & bench, unsigned long long iterations) {
	Trie test;
	auto dis = rng.getDistribution(0, 65535);
	
	for(size_t i=0; i<40000; ++i) {
		test.insert(dis());
	}
	
	bench.start();
	for(size_t i=0; i<iterations; ++i) {
		test.erase(dis());
	}
	bench.stop();
}

void bench5(Benchmark & bench, unsigned long long iterations) {
	Bitvector test;
	Bitvector test2;
	auto dis = rng.getDistribution(0, 65535);
	
	for(size_t i=0; i<30000; ++i) {
		test.insert(dis());
		test2.insert(dis());
	}
	
	bench.start();
	for(size_t i=0; i<iterations; ++i) {
		Bitvector * r = test.union_op(test2);
		r->empty();
		delete r;
	}
	bench.stop();
}

void bench6(Benchmark & bench, unsigned long long iterations) {
	Trie test;
	Trie test2;
	auto dis = rng.getDistribution(0, 65535);
	
	for(size_t i=0; i<30000; ++i) {
		test.insert(dis());
		test2.insert(dis());
	}
	
	bench.start();
	for(size_t i=0; i<iterations; ++i) {
		Trie * r = test.union_op(test2);
		r->empty();
		delete r;
	}
	bench.stop();
}

void bench7(Benchmark & bench, unsigned long long iterations) {
	Bitvector test;
	Bitvector test2;
	auto dis = rng.getDistribution(0, 65535);
	
	for(size_t i=0; i<30000; ++i) {
		test.insert(dis());
		test2.insert(dis());
	}
	
	bench.start();
	for(size_t i=0; i<iterations; ++i) {
		Bitvector * r = test.intersection_op(test2);
		r->empty();
		delete r;
	}
	bench.stop();
}

void bench8(Benchmark & bench, unsigned long long iterations) {
	Trie test;
	Trie test2;
	auto dis = rng.getDistribution(0, 65535);
	
	for(size_t i=0; i<30000; ++i) {
		test.insert(dis());
		test2.insert(dis());
	}
	
	bench.start();
	for(size_t i=0; i<iterations; ++i) {
		Trie * r = test.intersection_op(test2);
		r->empty();
		delete r;
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
		
		BenchmarkSet bench_1("Bitvector insert", 	bench1, 300s);
		BenchmarkSet bench_2("Trie insert", 		bench2, 300s);
		BenchmarkSet bench_3("Bitvector erase", 	bench3, 300s);
		BenchmarkSet bench_4("Trie erase", 			bench4, 300s);
		BenchmarkSet bench_5("Bitvector union", 	bench5, 300s);
		BenchmarkSet bench_6("Trie union", 			bench6, 300s);
		BenchmarkSet bench_7("Bitvector intersect", bench7, 300s);
		BenchmarkSet bench_8("Trie intersect", 		bench8, 300s);
		
		start_time = std::chrono::high_resolution_clock().now();
		
		// Basic sizes
		bench_1.run(time, 32000, precision);
		bench_2.run(time, 32000, precision);
		bench_3.run(time, 32000, precision);
		bench_4.run(time, 32000, precision);
		bench_5.run(time, 32000, precision);
		bench_6.run(time, 32000, precision);
		bench_7.run(time, 32000, precision);
		bench_8.run(time, 32000, precision);
		
		mid_time = std::chrono::high_resolution_clock().now();
		
		std::cout << bench_1.getStats() << std::flush;
		std::cout << bench_2.getStats() << std::flush;
		std::cout << bench_3.getStats() << std::flush;
		std::cout << bench_4.getStats() << std::flush;
		std::cout << bench_5.getStats() << std::flush;
		std::cout << bench_6.getStats() << std::flush;
		std::cout << bench_7.getStats() << std::flush;
		std::cout << bench_8.getStats() << std::endl;
		
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
