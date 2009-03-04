CXX = g++
CXXFLAGS = -std=c++0x -g -O2 -pedantic -ansi -Wall -Wextra -Werror -Wconversion -Wshadow -Wcast-qual -Wwrite-strings 
CPPFLAGS = 
INCS = 
DEFS = 
LIBS = -lboost_system -lboost_thread -lboost_regex
TESTLIBS = -lUnitTest++

all: tags tests robot

tags: *.cpp *.hpp
	ctags -R 

.PHONY: tests
tests: test_robot 
	./test_robot

test_robot: test_robot.o robot.o client_buffer.o
	$(CXX) -o $@ $^ $(LIBS) $(TESTLIBS)

robot: main.o robot.o telnet_client.o client_buffer.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS)

%.o: %.cpp
	$(CXX) -o $@ -c $(CPPFLAGS) $(CXXFLAGS) $(INCS) $(DEFS) $< 

.PHONY: clean
clean: 
	rm -f *.o *~
