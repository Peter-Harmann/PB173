#include <iostream>
#include "Benchmark.h"

using namespace std;
using namespace std::chrono_literals;


void bench1(Benchmark & bench) {
	long long a = 65535;
	bench.start();
	for (size_t i = 0; i < 1000000; ++i) {
		a = a * a;
	}
	bench.stop();
}


int main() {
	try {
		Benchmark bench_1("Benchmark 1", bench1);

		bench_1.run(3s);

		std::cout << bench_1.getStats() << std::endl;
	}
	catch (const std::exception & e) {
		std::cout << e.what() << std::endl;
	}
    return 0;
}
