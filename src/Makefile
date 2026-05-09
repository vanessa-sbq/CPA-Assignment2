# CXX=g++
# CC=gcc

CXXFLAGS+=-Wall -Wextra -Werror
CXXFLAGS+=-O3
CXXFLAGS+=-fopenmp

SRC=lu_factorization.cpp
TARGET=Assignment2

$(TARGET): $(SRC) Makefile
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
