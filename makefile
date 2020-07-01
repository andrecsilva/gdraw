CXX=g++
CXXFLAGS=-O3 -g
LDIR=-L/usr/lib/x86_64-linux-gnu/
STD=-std=c++1z
#Logging, if needed
#DMACRO=-DBOOST_LOG_DYN_LINK
#LDLIBS=-lboost_graph -lboost_regex -lpthread -lboost_log -lboost_system
LDLIBS=-lboost_graph -lboost_regex 
LDBOOSTTEST=-lboost_system -lboost_thread -lboost_unit_test_framework
RM=rm -f

xnumber: xnumber.cpp include/gdraw/xnumber.hpp include/gdraw/util.hpp include/gdraw/io.hpp 
	$(CXX) $(CXXFLAGS) $(STD) -o $@ xnumber.cpp $(LDIR) $(LDLIBS)

finddpc: finddpc.cpp include/gdraw/pplane.hpp include/gdraw/io.hpp include/gdraw/draw.hpp
	$(CXX) $(CXXFLAGS) $(STD) -o $@ finddpc.cpp $(LDIR) $(LDLIBS)

findppembedding: findppembedding.cpp include/gdraw/pplane.hpp include/gdraw/io.hpp include/gdraw/draw.hpp
	$(CXX) $(CXXFLAGS) $(STD) -o $@ findppembedding.cpp $(LDIR) $(LDLIBS)

unit_test: xnumber.hpp util.hpp unit_test.cpp
	$(CXX) -o $@ unit_test.cpp $(LDIR) $(LDLIBS) $(LDBOOSTTEST)

test: include/gdraw/coordinates.hpp include/gdraw/pplane.hpp include/gdraw/xnumber.hpp include/gdraw/util.hpp include/gdraw/draw.hpp include/gdraw/io.hpp test.cpp
	$(CXX) -std=c++1z -o quick_test -g test.cpp $(LDIR) $(LDLIBS)

.PHONY: clean
.PHONY: quick_test
.PHONY: run_unit_test

quick_test : test
	./quick_test

run_unit_test : unit_test
	./unit_test

clean: 
	$(RM) test unit_test main
