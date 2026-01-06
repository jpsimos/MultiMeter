CXX ?= g++
CC ?= gcc
CXXFLAGS ?= -O3 --std=c++20
CPPFLAGS ?= $(CXXFLAGS)
LDFLAGS += -lsqlite3
LDLIBS += -lsqlite3
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

all: $(OBJECTS) MultiMeter

MultiMeter: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o MultiMeter $(OBJECTS) $(LDLIBS)

clean:
	rm -f $(OBJECTS) MultiMeter

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
