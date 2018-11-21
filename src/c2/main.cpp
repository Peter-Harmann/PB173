
#include <iostream>
#include <vector>
#include <utility>
#include <cmath>




using DataSet = std::vector<std::pair<double,double>>;

double compute(const DataSet & data, double a) {
	double t = 0.0;
	for(const auto & i : data) {
		double d = i.second - a * i.first;
		t += d * d;
	}
	return t;
}

// y = a * x^3
double fit(const DataSet & data) {
	double last = 0;
	double sum_sq = compute(data, last);
	double step = 100;
	while(std::fabs(step) >= 0.00001) {
		double up = compute(data, last + step);
		if(up < sum_sq) {
			sum_sq = up;
			last = last + step;
		}
		else {
			double down = compute(data, last - step);
			if(down < sum_sq) {
				sum_sq = down;
				last = last +- step;
			}
		}
		step /= 2;
	}
	return last;
}




int main(int argc, char ** argv) {
	DataSet data;
	
	double x, y;
	std::cin >> x >> y;
	while(!std::cin.eof()) {
		std::cin >> x >> y;
		data.push_back(std::make_pair((x * x * x), y));
	}
	
	double a = fit(data);
	std::cout << a << std::endl;
	return 0;
}


