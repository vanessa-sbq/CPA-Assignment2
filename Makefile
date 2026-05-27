# Made a wrapper because the cmake commands are annoying to write
SYCL_COMPILER?=acpp
CXX=$(SYCL_COMPILER)

.PHONY: build run clean

MAKEFLAGS+=--no-print-directory

build:
	@cmake --build ./build --config Release --target all -j --

run: build
	@bin/Assignment2

config: clean
	@rm -rf build
	@cmake -DCMAKE_BUILD_TYPE:STRING=Release --no-warn-unused-cli -S ./src -B ./build -DCMAKE_CXX_COMPILER=$(CXX)

clean:
	@cmake --build ./build --target clean
