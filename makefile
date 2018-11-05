

CC=gcc
CXX=g++
CFLAGS=-I.
CXXFLAGS=-std=c++14 -O3 -Wall -I.

HEADERS = src/hw1/Benchmark.h
HW1_OBJECTS = src/hw1/main.o src/hw1/Benchmark.o
HW2_OBJECTS = src/hw2/main.o src/hw1/Benchmark.o
HW3_OBJECTS = src/hw3/main.o src/hw1/Benchmark.o

EX2_OBJECTS = src/ex2/main.o src/hw1/Benchmark.o
EX5_OBJECTS = src/ex5/bfs.o src/hw1/Benchmark.o

C1_OBJECTS = src/c1/main.o src/hw1/Benchmark.o

all: hw1 hw2 ex2

%.o: %.cpp $(HEADERS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

ex2: $(EX2_OBJECTS)
	$(CXX) -o ex2 $(EX2_OBJECTS)
	
ex5: $(EX5_OBJECTS)
	$(CXX) -o ex5 $(EX5_OBJECTS)
	
c1: $(C1_OBJECTS)
	$(CXX) -o c1 $(C1_OBJECTS)
	
hw1: $(HW1_OBJECTS)
	$(CXX) -o hw1 $(HW1_OBJECTS)

hw2: $(HW2_OBJECTS)
	$(CXX) -o hw2 $(HW2_OBJECTS)

hw3: $(HW3_OBJECTS)
	$(CXX) -o hw3 $(HW3_OBJECTS)


