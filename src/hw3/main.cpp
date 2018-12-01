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
#include <memory>

using namespace std;
using namespace std::literals::chrono_literals;

struct RNG {
	std::mt19937 generator;
	RNG() {
		generator.seed(static_cast<unsigned int>(std::chrono::high_resolution_clock().now().time_since_epoch().count()));
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

template <int level>
class Node {
	std::array<unique_ptr<Node<level-1>>, 16> children;
	
public:
	Node() {}
	Node(const Node<level> & other) { for(size_t i = 0; i < 16; ++i) children[i].reset(!other.children[i] ? nullptr : new Node<level-1>(*other.children[i])); }
	Node(Node<level> && other) { for(size_t i = 0; i < 16; ++i) { children[i] = other.children[i]; other.children[i] = nullptr; } }
	~Node() {}
	
	bool empty() const {
		for(size_t i = 0; i < 16; ++i) if(children[i] && !children[i]->empty()) return false;
		return true;
	}
	void insert(uint16_t num) {
		if(!children[num & 0xF]) {
			children[num & 0xF] = make_unique<Node<level-1>>();
			children[num & 0xF]->insert(num >> 4);
		}
		else children[num & 0xF]->insert(num >> 4);
	}
	void erase(uint16_t num) {
		if(children[num & 0xF]) {
			children[num & 0xF]->erase(num >> 4);
			if(children[num & 0xF]->empty()) {
				children[num & 0xF].reset();
			}
		}
	}
	unique_ptr<Node<level>> operator|(const Node<level> & o) {
		unique_ptr<Node<level>> res = make_unique<Node<level>>();
		for(size_t i = 0; i < 16; ++i) {
			if(children[i]) {
				if(o.children[i]) res->children[i] = *children[i] | *o.children[i];
				else res->children[i] = make_unique<Node<level-1>>(*children[i]);
			}
			else {
				if(o.children[i]) res->children[i] = make_unique<Node<level-1>>(*o.children[i]);
			}
		}
		return res;
	}
	unique_ptr<Node<level>> operator&(const Node<level> & o) {
		unique_ptr<Node<level>> res = make_unique<Node<level>>();
		for(size_t i = 0; i < 16; ++i) {
			if(children[i] && o.children[i]) res->children[i] = *children[i] & *o.children[i];
		}
		return res;
	}
};

template<>
class Node<1> {
	std::bitset<16> number;
	
public:
	Node() {}
	Node(const Node<1> & other) : number(other.number) {}
	Node(Node<1> && other) : number(std::move(other.number)) {}
	~Node() {}
	
	bool empty() const { return number.any(); }
	void insert(uint16_t num) { number[num & 0xF] = true; }
	void erase(uint16_t num) { number[num & 0xF] = false; }
	unique_ptr<Node<1>> operator|(const Node<1> & o) {
		unique_ptr<Node<1>> res = make_unique<Node<1>>();
		res->number = number | o.number;
		return res;
	}
	unique_ptr<Node<1>> operator&(const Node<1> & o) {
		unique_ptr<Node<1>> res = make_unique<Node<1>>();
		res->number = number & o.number;
		return res;
	}
};

class Trie {
	unique_ptr<Node<4>> root;
public:
	bool empty() const { return root->empty(); }

	Trie() : root(new Node<4>()) {}
	Trie(unique_ptr<Node<4>> && n) : root(std::move(n)) {}
	Trie(const Trie & o) : root(make_unique<Node<4>>(*o.root)) {}
	Trie(Trie && o) : root(o.root.release()) {}
	~Trie() {}
	Trie & operator=(const Trie &) = delete;
	Trie & operator=(Trie &&) = delete;

	void insert(uint16_t num) { root->insert(num); }
	void erase(uint16_t num) { root->erase(num); }
	
	unique_ptr<Trie> union_op(const Trie & o) { return make_unique<Trie>(*root | *o.root); }
	unique_ptr<Trie> intersection_op(const Trie & o) { return make_unique<Trie>(*root & *o.root); }
};

const size_t single_iterations_bitvector_insert = 150000;
const size_t single_iterations_trie_insert = 75000;
const size_t single_iterations_bitvector_erase = 150000;
const size_t single_iterations_trie_erase = 120000;
const size_t single_iterations_bitvector_union = 3000;
const size_t single_iterations_trie_union = 5000;
const size_t single_iterations_bitvector_intersect = 3000;
const size_t single_iterations_trie_intersect = 60000;

size_t trie_union_increment(unsigned long long iter) {
	if(iter <= 32) 		return 32;
	if(iter <= 64) 		return 64;
	if(iter <= 128)		return 108;
	if(iter <= 256)		return 187;
	if(iter <= 512)		return 281;
	if(iter <= 1024)	return 432;
	if(iter <= 2048)	return 617;
	if(iter <= 4086)	return 752;
	if(iter <= 8192)	return 752;
	if(iter <= 16384)	return 752;
	return 752;
}

void bench1(Benchmark & bench, unsigned long long iterations) {
	Bitvector test;
	auto dis = rng.getDistribution(0, 65535);
	
	bench.start();
	for(size_t i=0; i < single_iterations_bitvector_insert; ++i) {
		test.insert(dis());
	}
	bench.stop();
	bench.set_repeats(single_iterations_bitvector_insert);
}

void bench2(Benchmark & bench, unsigned long long iterations) {
	Trie test;
	auto dis = rng.getDistribution(0, 65535);
	
	bench.start();
	for(size_t i=0; i < single_iterations_trie_insert; ++i) {
		test.insert(dis());
	}
	bench.stop();
	bench.set_repeats(single_iterations_trie_insert);
}

void bench3(Benchmark & bench, unsigned long long iterations) {
	Bitvector test;
	auto dis = rng.getDistribution(0, 65535);
	
	for(size_t i=0; i<iterations; ++i) {
		test.insert(dis());
	}
	
	bench.start();
	for(size_t i=0; i < single_iterations_bitvector_erase; ++i) {
		test.erase(dis());
	}
	bench.stop();
	bench.set_repeats(single_iterations_bitvector_erase);
}

void bench4(Benchmark & bench, unsigned long long iterations) {
	Trie test;
	auto dis = rng.getDistribution(0, 65535);
	
	for(size_t i=0; i<iterations; ++i) {
		test.insert(dis());
	}
	
	bench.start();
	for(size_t i=0; i < single_iterations_trie_erase; ++i) {
		test.erase(dis());
	}
	bench.stop();
	bench.set_repeats(single_iterations_trie_erase);
}

void bench5(Benchmark & bench, unsigned long long iterations) {
	Bitvector test;
	Bitvector test2;
	auto dis = rng.getDistribution(0, 65535);
	
	for(size_t i=0; i<iterations; ++i) {
		test.insert(dis());
		test2.insert(dis());
	}
	
	bench.start();
	for(size_t i=0; i < single_iterations_bitvector_union; ++i) {
		Bitvector * r = test.union_op(test2);
		r->empty();
		delete r;
	}
	bench.stop();
	bench.set_repeats(single_iterations_bitvector_union);
}

void bench6(Benchmark & bench, unsigned long long iterations) {
	Trie test;
	Trie test2;
	auto dis = rng.getDistribution(0, 65535);
	
	for(size_t i=0; i<iterations; ++i) {
		test.insert(dis());
		test2.insert(dis());
	}
	
	size_t increment = trie_union_increment(iterations);
	size_t iter_count = 0;
	bench.start();
	for(size_t i=0; i < single_iterations_trie_union; i += increment) {
		unique_ptr<Trie> r = test.union_op(test2);
		r->empty();
		++iter_count;
	}
	bench.stop();
	bench.set_repeats(iter_count);
}

void bench7(Benchmark & bench, unsigned long long iterations) {
	Bitvector test;
	Bitvector test2;
	auto dis = rng.getDistribution(0, 65535);
	
	for(size_t i=0; i<iterations; ++i) {
		test.insert(dis());
		test2.insert(dis());
	}
	
	bench.start();
	for(size_t i=0; i < single_iterations_bitvector_intersect; ++i) {
		Bitvector * r = test.intersection_op(test2);
		r->empty();
		delete r;
	}
	bench.stop();
	bench.set_repeats(single_iterations_bitvector_intersect);
}

void bench8(Benchmark & bench, unsigned long long iterations) {
	Trie test;
	Trie test2;
	auto dis = rng.getDistribution(0, 65535);
	
	for(size_t i=0; i<iterations; ++i) {
		test.insert(dis());
		test2.insert(dis());
	}
	
	size_t iter_count = 0;
	bench.start();
	for(size_t i = 0; i < single_iterations_trie_intersect; i += iterations) {
		unique_ptr<Trie> r = test.intersection_op(test2);
		r->empty();
		++iter_count;
	}
	bench.stop();
	bench.set_repeats(iter_count);
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
		bench_1.run(time, 0, precision);
		bench_2.run(time, 0, precision);
		bench_3.run(time, bin_sequence(32,10), precision);
		bench_4.run(time, bin_sequence(32,10), precision);
		bench_5.run(time, bin_sequence(32,10), precision);
		bench_6.run(time, bin_sequence(32,10), precision);
		bench_7.run(time, bin_sequence(32,10), precision);
		bench_8.run(time, bin_sequence(32,10), precision);
		
		mid_time = std::chrono::high_resolution_clock().now();
		
		bool extra_stats = false;
		std::cout << bench_1.getStats(extra_stats) << std::flush;
		std::cout << bench_2.getStats(extra_stats) << std::flush;
		std::cout << bench_3.getStats(extra_stats) << std::flush;
		std::cout << bench_4.getStats(extra_stats) << std::flush;
		std::cout << bench_5.getStats(extra_stats) << std::flush;
		std::cout << bench_6.getStats(extra_stats) << std::flush;
		std::cout << bench_7.getStats(extra_stats) << std::flush;
		std::cout << bench_8.getStats(extra_stats) << std::endl;
		
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
