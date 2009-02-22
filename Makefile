CXX = g++
CXXFLAGS = -g -O2 -pedantic -ansi -Wall -Wextra -Werror -Wconversion -Wshadow -Wcast-qual -Wwrite-strings 
CPPFLAGS = 
INCS = 
DEFS = 
LIBS = -lboost_system -lboost_thread

all: tags robot

tags: *.cpp *.hpp
	ctags -R 

robot: main.o robot.o telnet_client.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS)

%.o: %.cpp
	$(CXX) -o $@ -c $(CPPFLAGS) $(CXXFLAGS) $(INCS) $(DEFS) $< 

.PHONY: clean
clean: 
	rm -f *.o
