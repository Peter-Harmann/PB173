
#include <iostream>
#include <vector>
#include <cstdlib>

static inline bool isPrime(int n, const std::vector<int> & primes) {
	for(int i : primes) {
		if(i * i > n) return true;
		if(n % i == 0) return false;
	}
	return true;
}

static int prime(int n) {
	std::vector<int> primes;
	primes.reserve(n+1);
	primes.push_back(2);
	primes.push_back(3);
	
	if(n == 1) return 2;
	if(n == 2) return 3;
	n-=2;
	for(int i=6;; i+=6) {
		if(isPrime(i-1, primes)) {
			if(--n == 0) return i-1;
			primes.push_back(i-1);
		}
		if(isPrime(i+1, primes)) {
			if(--n == 0) return i+1;
			primes.push_back(i+1);
		}
	}
	// 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
}



int main(int argc, char ** argv) {
	if(argc != 2) {
		return 1;
	}
	
	int n = atoi (argv[1]);
	
	std::cout << n << "th prime: " << prime(n) << std::endl;
	return 0;
}




