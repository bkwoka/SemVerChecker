CXX = g++
CXXFLAGS = -I. -Wall -std=c++11 -DARDUINO
COVERAGE_FLAGS = --coverage -fprofile-arcs -ftest-coverage

all: test

test: run_tests
	./run_tests

run_tests: run_tests.cpp Arduino.h ../src/SemVerChecker.cpp ../src/SemVerChecker.h
	$(CXX) $(CXXFLAGS) -o run_tests run_tests.cpp

coverage: run_tests.cpp Arduino.h ../src/SemVerChecker.cpp ../src/SemVerChecker.h
	$(CXX) $(CXXFLAGS) $(COVERAGE_FLAGS) -o run_tests run_tests.cpp
	./run_tests
	lcov --capture --directory . --output-file coverage.info --base-directory .. --ignore-errors empty
	lcov --remove coverage.info '*/tests/*' '*/Arduino.h' '/usr/*' --output-file coverage.info --ignore-errors unused

clean:
	rm -f run_tests *.gcda *.gcno coverage.info
