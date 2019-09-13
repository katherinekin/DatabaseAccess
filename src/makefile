CXX := g++
CXXFLAGS += -std=c++11

.PHONY = all send encode

all: access

access: accessdatabase.o
	$(CXX) -o access accessdatabase.o -pthread

accessdatabase.o:
	$(CXX) -c accessdatabase.cpp 

clean: rm -f access accessdatabase.o