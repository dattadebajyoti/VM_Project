OUT			= ./build
MK_DIR		= mkdir -p $(OUT)

JITBUILDER_DIR = ../../lib/omr/jitbuilder/release/cpp

CXX      = /usr/bin/g++
CXXFLAGS	= --std=c++0x -fno-rtti -fPIC -Wno-write-strings -O3
HEADERS		= -I. -I../../lib/omr/include_core -I$(JITBUILDER_DIR)/include

JITBUILDER_LD	= -L$(JITBUILDER_DIR) -ljitbuilder -ldl 

main: tpch-q1.o main.o
	$(MK_DIR)
	$(CXX) $(CXXFLAGS) -o $(OUT)/$@ tpch-q1.o main.o $(JITBUILDER_LD)

tpch-q1.o: tpch-q1.cpp
	$(CXX) $(CXXFLAGS) $(HEADERS) -c $< -o $@

main.o: main.cpp
	$(CXX) $(CXXFLAGS) $(HEADERS) -c $< -o $@

