#include <iostream>
#include "../hw1/Benchmark.h"

#include <random>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <bitset>
#include <string>
#include <cstring>
#include <array>

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

bool strfind1(const char * str, const char * substr) {
	while(*str != '\0') {
		const char * str_pos = str;
		const char * substr_pos = substr;
		while(*substr_pos != '\0' && *str_pos == *substr_pos) {
			++str_pos;
			++substr_pos;
		}
		if(*substr_pos == '\0') return true;
		++str;
	}
	return false;
}

void compute_T(std::vector<int> & T, const char * substr) {
	T.resize(strlen(substr)+1);
	T[0] = -1;
	int i = 1;
	int j = 0;
	
	while(substr[i] != '\0') {
		if(substr[i] == substr[j]) {
			T[i] = T[j];
			++i;
			++j;
		}
		else {
			T[i] = j;
			j = T[j];
			
			while(j >= 0 && substr[i] != substr[j]) {
                j = T[j];
			}
            ++i;
			++j;
		}
	}
}

bool strfind2_find(const char * str, const char * substr, const std::vector<int> & T) {	
	const char * substr_pos = substr;
	while(*str != '\0') {
		if(*str == *substr_pos) {
			++substr_pos;
			++str;
			if(*substr_pos == '\0') return true;
		}
		else {
			substr_pos = substr + T[substr_pos - substr];
			if(substr_pos < substr) {
				++substr_pos;
				++str;
			}
		}
	}
	return false;
}

bool strfind2(const char * str, const char * substr) {
	std::vector<int> T;
	compute_T(T, substr);
	return strfind2_find(str, substr, T);
}

struct DFA_Node {
	std::array<int,128> data;
	
	DFA_Node() {
		for(size_t i=0; i<128; ++i) data[i] = 0;
		data[0] = -1;
	}
	
	void inherit(const DFA_Node & o, size_t except) {
		for(size_t i=0; i<128; ++i) {
			if(i == except) continue;
			data[i] = o.data[i];
		}
	}
	
	int & operator[](size_t i) { return data[i]; }
	int operator[](size_t i) const { return data[i]; }
};	
using DFA = std::vector<DFA_Node>;

void compute_DFA(DFA & dfa, const char * substr) {
	size_t len = strlen(substr);
	dfa.clear();
	dfa.resize(len);
	
	for(size_t i=0; i<len-1; ++i) dfa[i][substr[i]] = i+1;
	dfa[len-1][substr[len-1]] = -2;
	if(substr[0] != substr[1]) dfa[1][substr[0]] = 1;
	
	--len;
	size_t i = 1;
	size_t j = 0;
	while(i < len) {
		if(substr[i] == substr[j]) {
			dfa[i+1].inherit(dfa[j+1], substr[i+1]);
			++i;
			++j;
		}
		else {
			j = dfa[j][substr[i]];
			if(substr[i+1] != substr[j]) dfa[i+1][substr[j]] = j+1;
			++i;
		}
	}
}

bool strfind3_find(const char * str, const DFA & dfa) {
	int s = 0;
	do {
		s = dfa[s][*str];
		++str;
	}
	while(s >= 0);
	return s == -2;
}

bool strfind3(const char * str, const char * substr) {
	DFA dfa;
	compute_DFA(dfa, substr);
	return strfind3_find(str, dfa);
}

void strfind3_test(const char * str, const char * substr) {
	std::cout << "strfind3(\"" << str << "\", \"" << substr << "\") = " << strfind3(str, substr) << std::endl;
}

volatile int dump = 0;
std::string global_str;
std::string global_substr;

size_t repeat_each = 1000;

void bench1(Benchmark & bench, unsigned long long iterations) {
	int res = 0;
	char bu = global_substr[iterations];
	char bu2 = global_substr[iterations-1];
	global_substr[iterations] = '\0';
	global_substr[iterations-1] = 'Z'; // Make sure nothing matches!
	
	bench.start();
	for(size_t i=0; i<repeat_each; ++i) {
		res += strfind1(global_str.c_str(), global_substr.c_str());
	}
	bench.stop();
	bench.set_repeats(repeat_each);
	
	global_substr[iterations] = bu;
	global_substr[iterations-1] = bu2;
	dump += res;
}

void bench2(Benchmark & bench, unsigned long long iterations) {
	int res = 0;
	char bu = global_substr[iterations];
	char bu2 = global_substr[iterations-1];
	global_substr[iterations] = '\0';
	global_substr[iterations-1] = 'Z'; // Make sure nothing matches!
	
	bench.start();
	for(size_t i=0; i<repeat_each; ++i) {
		res += strfind2(global_str.c_str(), global_substr.c_str());
	}
	bench.stop();
	bench.set_repeats(repeat_each);
	
	global_substr[iterations] = bu;
	global_substr[iterations-1] = bu2;
	dump += res;
}

void bench3(Benchmark & bench, unsigned long long iterations) {
	int res = 0;
	char bu = global_substr[iterations];
	char bu2 = global_substr[iterations-1];
	global_substr[iterations] = '\0';
	global_substr[iterations-1] = 'Z'; // Make sure nothing matches!
	
	bench.start();
	std::vector<int> T;
	compute_T(T, global_substr.c_str());
	for(size_t i=0; i<repeat_each; ++i) {
		res += strfind2_find(global_str.c_str(), global_substr.c_str(), T);
	}
	bench.stop();
	bench.set_repeats(repeat_each);
	
	global_substr[iterations] = bu;
	global_substr[iterations-1] = bu2;
	dump += res;
}

void bench4(Benchmark & bench, unsigned long long iterations) {
	int res = 0;
	char bu = global_substr[iterations];
	char bu2 = global_substr[iterations-1];
	global_substr[iterations] = '\0';
	global_substr[iterations-1] = 'Z'; // Make sure nothing matches!
	
	bench.start();
	for(size_t i=0; i<repeat_each; ++i) {
		res += strfind3(global_str.c_str(), global_substr.c_str());
	}
	bench.stop();
	bench.set_repeats(repeat_each);
	
	global_substr[iterations] = bu;
	global_substr[iterations-1] = bu2;
	dump += res;
}

void bench5(Benchmark & bench, unsigned long long iterations) {
	int res = 0;
	char bu = global_substr[iterations];
	char bu2 = global_substr[iterations-1];
	global_substr[iterations] = '\0';
	global_substr[iterations-1] = 'Z'; // Make sure nothing matches!
	
	bench.start();
	DFA dfa;
	compute_DFA(dfa, global_substr.c_str());
	for(size_t i=0; i<repeat_each; ++i) {
		res += strfind3_find(global_str.c_str(), dfa);
	}
	bench.stop();
	bench.set_repeats(repeat_each);
	
	global_substr[iterations] = bu;
	global_substr[iterations-1] = bu2;
	dump += res;
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
		
		global_str.resize(100001);
		global_substr.resize(16385);
		global_str[100000] = 0;
		global_str[16384] = 0;
		
		auto dist = rng.getDistribution('a', 'z');
		for(size_t i = 0; i<100000; ++i) global_str[i] = dist();
		for(size_t i = 0; i<16384; ++i) global_substr[i] = dist();
		
		BenchmarkSet bench_1 ("Naive search", 				bench1, 300s);
		BenchmarkSet bench_2 ("KMP search full", 			bench2, 300s);
		BenchmarkSet bench_3 ("KMP search PC", 				bench3, 300s);
		BenchmarkSet bench_4 ("DFA search full", 			bench4, 300s);
		BenchmarkSet bench_5 ("DFA search PC", 				bench5, 300s);
		BenchmarkSet bench_6 ("Naive search a-only", 		bench1, 300s);
		BenchmarkSet bench_7 ("KMP search a-only", 			bench2, 300s);
		BenchmarkSet bench_8 ("KMP search PC a-only", 		bench3, 300s);
		BenchmarkSet bench_9 ("DFA search a-only", 			bench4, 300s);
		BenchmarkSet bench_0 ("DFA search PC a-only", 		bench5, 300s);
		
		start_time = std::chrono::high_resolution_clock().now();
		
		// Basic sizes
		bench_1.run(time, bin_sequence(2, 14), precision);
		bench_2.run(time, bin_sequence(2, 14), precision);
		bench_3.run(time, bin_sequence(2, 14), precision);
		bench_4.run(time, bin_sequence(2, 14), precision);
		bench_5.run(time, bin_sequence(2, 14), precision);
		
		for(size_t i = 0; i<100000; ++i) global_str[i] = 'a';
		for(size_t i = 0; i<16384; ++i) global_substr[i] = 'a';
	
		bench_6.run(time, bin_sequence(2, 5),  precision);
		bench_7.run(time, bin_sequence(2, 14), precision);
		bench_8.run(time, bin_sequence(2, 14), precision);
		bench_9.run(time, bin_sequence(2, 14), precision);
		bench_0.run(time, bin_sequence(2, 14), precision);
		
		repeat_each = 20;
		bench_6.run(time, bin_sequence(64, 5),  precision);
		
		repeat_each = 2;
		bench_6.run(time, bin_sequence(2048, 4),  precision);
		
		mid_time = std::chrono::high_resolution_clock().now();
		
		std::cout << bench_1.getStats() << std::flush;
		std::cout << bench_2.getStats() << std::flush;
		std::cout << bench_3.getStats() << std::flush;
		std::cout << bench_4.getStats() << std::flush;
		std::cout << bench_5.getStats() << std::flush;
		std::cout << bench_6.getStats() << std::flush;
		std::cout << bench_7.getStats() << std::flush;
		std::cout << bench_8.getStats() << std::flush;
		std::cout << bench_9.getStats() << std::flush;
		std::cout << bench_0.getStats() << std::flush;
		std::cout << std::endl;
		
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
		
		// Prevent dump from being optimized away, also check no matches found
		if(dump != 0) {
			std::cerr << "Error!" << std::endl;
		}
	}
	catch (const std::exception & e) {
		std::cout << e.what() << std::endl;
	}
    return 0;
}
