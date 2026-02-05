CXX := clang++-18
CXXFLAGS := -g -I/usr/lib/llvm-18/include -fPIC -fvisibility-inlines-hidden -Werror=date-time -std=c++20 -Wall -W -Wno-unused-parameter -Wwrite-strings -Wcast-qual -Wno-missing-field-initializers -pedantic -Wno-long-long -Wno-uninitialized -Wdelete-non-virtual-dtor -Wno-comment -ffunction-sections -fdata-sections -O2 -DNDEBUG -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Iinclude/ -I.
LLVMLDFLAGS := $(shell llvm-config-18 --ldflags --system-libs --libs) $(LDFLAGS)

SOURCES := $(wildcard *.cpp) $(wildcard src/*.cpp) $(wildcard Generator/*.cpp)
OBJECTS := $(SOURCES:.cpp=.o)

CLANGLIBS := -lclangFrontend -lclangSerialization -lclangDriver -lclangTooling -lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST -lclangASTMatchers -lclangLex -lclangBasic -lclangRewrite -lclangRewriteFrontend -lclangSupport

askeleton: $(OBJECTS)
	$(CXX) -o $@ $^ $(CLANGLIBS) $(LLVMLDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

install: askeleton
	cp askeleton /usr/local/bin

clean:
	rm -f -r *.o Generator/*.o src/*.o askeleton

.PHONY: install clean
