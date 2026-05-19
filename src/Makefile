# CXX=g++
# CC=gcc

CXXFLAGS+=-Wall -Wextra -Werror
CXXFLAGS+=-O3 # TODO: Should this be used?
CXXFLAGS+=-fopenmp
CXXFLAGS+=-std=c++17

SRC=main.cpp
SRC+=matrix.cpp
SRC+=perf.cpp
SRC+=lu_fact.cpp

TARGET=Assignment2

$(TARGET): $(SRC) Makefile
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
