

SOURCES = flatalg_tests.cpp main.cpp

HCONLIB_ROOT = ..
HCONLIB_INCLUDE = $(HCONLIB_ROOT)/include
HCONLIB_LIB = $(HCONLIB_ROOT)/lib


all: flatalg_test_avx flatalg_test

flatalg_test_avx: $(SOURCES)
	g++ -o $@ $^ -I $(HCONLIB_INCLUDE) -L $(HCONLIB_LIB) -mavx -std=c++1z -O3

flatalg_test: $(SOURCES)
	g++ -o $@ $^ -I $(HCONLIB_INCLUDE) -L $(HCONLIB_LIB) -std=c++1z -O3
