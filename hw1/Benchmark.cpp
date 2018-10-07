
#include "Benchmark.h"





void Sample::addSample(const std::chrono::nanoseconds & time) {
	times.push_back(time);
}

void Benchmark::start() {
	start_time = std::chrono::high_resolution_clock().now();
}

void Benchmark::stop() {
	end_time = std::chrono::high_resolution_clock().now();
}

void Benchmark::run(std::chrono::seconds time) {
	to_run = time;
	bench_start = std::chrono::system_clock().now();

	while (repeat()) {
		func(*this);
		sample.addSample(end_time - start_time);
	}
}

bool Benchmark::repeat() const {
	return to_run > (std::chrono::system_clock().now() - bench_start);
}






