
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include "../hw1/Benchmark.h"

using namespace std::chrono_literals;

static inline bool isPrime(int n, const std::vector<int> & primes) {
	for(int i : primes) {
		if(i*i > n) return true;
		if(n % i == 0) return false;
	}
	return true;
}

static int prime(int n, int mid) {
	std::vector<int> primes;
	primes.reserve(n/2);
	mid += 1;
	
	if(n == 1) return 2;
	if(n == 2) return 3;
	n-=2;
	for(int i=6; ; i+=6) {
		if(isPrime(i-1, primes)) {
			if(--n == 0) return i-1;
			primes.push_back(i-1);
		}
		if(isPrime(i+1, primes)) {
			if(--n == 0) return i+1;
			primes.push_back(i+1);
		}
	}
	
	for(int i=6;; i+=6) {
		if(isPrime(i-1, primes)) {
			if(--n == 0) return i-1;
		}
		if(isPrime(i+1, primes)) {
			if(--n == 0) return i+1;
		}
	}
}

void bench1(Benchmark & bench, int iter) {
	bench.start();
	register double x = iter;
	register double y = std::log(x);
	register double top = x * (y + std::log(y));
	int mid = std::ceil(std::sqrt(top));
	std::cout << iter << "th prime: " << prime(iter, mid) << std::endl;
	bench.stop();
}

int main(int argc, char ** argv) {
	if(argc != 2) {
		return 1;
	}
	int n = atoi (argv[1]);
	
	Benchmark bench("a", bench1);
	
	bench.run(1s, n);
	
	std::cout << bench.getStats() << std::endl;
	return 0;
}




