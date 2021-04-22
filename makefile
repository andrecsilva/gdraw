CXX=g++
INCLUDE_DIR=include

LDIR=-L/usr/lib/x86_64-linux-gnu/
LDLIBS=-lboost_graph -lboost_regex 
LDBOOSTTEST=-lboost_system -lboost_thread -lboost_unit_test_framework

CXXFLAGS=-O3 -std=c++20 -MMD -MP -I$(INCLUDE_DIR) $(LDIR) $(LDLIBS)
#Logging, if needed
#DMACRO=-DBOOST_LOG_DYN_LINK
#LDLIBS=-lboost_graph -lboost_regex -lpthread -lboost_log -lboost_system
RM=rm -f
#SRC=$(wildcard *.cpp)
SRC=xnumber.cpp finddpc.cpp findppembedding.cpp

.PHONY: all
all: $(SRC:%.cpp=%)
	

$(SRC:%.cpp=%): % : %.cpp
	g++ $(CXXFLAGS) -o $@ $<

-include $(SRC:%.cpp=%.d)

unit_test: unit_test.cpp 
	$(CXX) $(CXXFLAGS) -g -o $@ unit_test.cpp $(LDBOOSTTEST)

test: 
	$(CXX) $(CXXFLAGS) -pg -o quick_test test.cpp $(LDIR) $(LDLIBS)

.PHONY: clean
.PHONY: quick_test
.PHONY: run_unit_test

quick_test: test
	./quick_test

run_unit_test: unit_test
	./unit_test

clean: 
	$(RM) *.o *.d
