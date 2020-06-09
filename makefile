CXX=g++
CXXFLAGS=-O3 -g
LDIR=-L/usr/lib/x86_64-linux-gnu/
#Logging, if needed
#DMACRO=-DBOOST_LOG_DYN_LINK
#LDLIBS=-lboost_graph -lboost_regex -lpthread -lboost_log -lboost_system
LDLIBS=-lboost_graph -lboost_regex 
LDBOOSTTEST=-lboost_system -lboost_thread -lboost_unit_test_framework
RM=rm -f

main: xnumber.hpp util.hpp main.cpp
	$(CXX) $(CXXFLAGS) -o $@ util.hpp xnumber.hpp main.cpp $(LDIR) $(LDLIBS)

unit_test: xnumber.hpp util.hpp unit_test.cpp
	$(CXX) -o $@ util.hpp xnumber.hpp unit_test.cpp $(LDIR) $(LDLIBS) $(LDBOOSTTEST)

test: pplane.hpp xnumber.hpp util.hpp io.hpp test.cpp
	$(CXX) -o $@ -g $(DMACRO) pplane.hpp util.hpp xnumber.hpp io.hpp test.cpp $(LDIR) $(LDLIBS)

.PHONY: clean
.PHONY: quick_test
.PHONY: run_unit_test

quick_test : test
	./test

run_unit_test : unit_test
	./unit_test

clean: 
	$(RM) test unit_test
