CXX = g++
CXXFLAGS = -I. -Wall -std=c++11

all: test

test: run_tests
	./run_tests

run_tests: run_tests.cpp Arduino.h ../src/SemVerChecker.cpp ../src/SemVerChecker.h
	$(CXX) $(CXXFLAGS) -o run_tests run_tests.cpp

clean:
	rm -f run_tests
