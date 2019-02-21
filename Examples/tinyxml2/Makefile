CXX := clang++ --std=c++11
RTTIFLAG := -fno-rtti

SOURCES = test_functions.cpp tests.cpp execution_tests.cpp tinyxml2.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXES = $(OBJECTS:.o=)

all: tinyxml2.o

tinyxml2.o: tinyxml2.cpp tinyxml2.h
	clang++ -c tinyxml2.cpp	-DDEBUG

%: %.o
	$(CXX) -o $@ $< 

test: tinyxml2.o tests.o execution_tests.o
	$(CXX) -o $@ $^ TestLib.a

test_functions.o: test_functions.cpp test_functions.h

tests.o:  tests.cpp tests.h test_functions.h tinyxml2.h

execution_tests.o: execution_tests.cpp test_functions.h tests.h tinyxml2.h 
	$(CXX) -c execution_tests.cpp

execute:
	./test > /dev/null 2>&1

clean:
	rm -f test $(OBJECTS)  *~


