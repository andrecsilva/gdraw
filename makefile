CXX=g++
INCLUDE_DIR=include

LDIR=-L/usr/lib/x86_64-linux-gnu/
LDLIBS=-lboost_graph -lboost_regex -larmadillo
LDBOOSTTEST=-lboost_system -lboost_thread -lboost_unit_test_framework 

CXXFLAGS=-std=c++20 -MMD -MP -I$(INCLUDE_DIR) $(LDIR) $(LDLIBS)
#Logging, if needed
#DMACRO=-DBOOST_LOG_DYN_LINK
#LDLIBS=-lboost_graph -lboost_regex -lpthread -lboost_log -lboost_system
RM=rm -f
#SRC=$(wildcard *.cpp)
SRC=xnumber.cpp finddpc.cpp findppembedding.cpp
TEST=$(shell find test -iname 'test_*.cpp')

.PHONY: all
all: $(SRC:%.cpp=%)
	

$(SRC:%.cpp=%): % : %.cpp
	$(CXX) $(CXXFLAGS) -O3 -o $@ $<

-include $(SRC:%.cpp=%.d)

$(TEST:%.cpp=%): % : %.cpp
	$(CXX) $(CXXFLAGS) -p -g -o $@.test $< && ./$@.test

-include $(TEST:%.cpp=%.d)

test: test.cpp
	$(CXX) $(CXXFLAGS) -p -g -o quick_test test.cpp 

#$(info "$(TEST)")

test_all: $(TEST:%.cpp=%)
	

.PHONY: clean
.PHONY: quick_test

quick_test: test
	./quick_test

clean: 
	$(RM) *.o *.d
	$(RM) test/*.test test/*.d
