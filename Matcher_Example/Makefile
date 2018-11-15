CXX := clang++ 
RTTIFLAG := -fno-rtti
CXXFLAGS := $(shell llvm-config --cxxflags) $(RTTIFLAG) --std=c++11 -fexceptions
LLVMLDFLAGS := $(shell llvm-config --ldflags --libs) -lz -lpthread -lffi -ltinfo -ldl -lm

SOURCES = astut.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXES = $(OBJECTS:.o=)
CLANGLIBS = \
	-lclangFrontend \
	-lclangSerialization \
	-lclangDriver \
	-lclangTooling \
	-lclangParse \
	-lclangSema \
	-lclangAnalysis \
	-lclangEdit \
	-lclangAST \
	-lclangASTMatchers \
	-lclangLex \
	-lclangBasic \
	-lclangRewrite

all: $(OBJECTS) $(EXES)
.PHONY: clean install

%: %.o
	$(CXX) -o $@ $< $(CLANGLIBS) $(LLVMLDFLAGS)

astut: astut.o
	$(CXX) -o $@ $^ $(CLANGLIBS) $(LLVMLDFLAGS)

clean:
	-rm -f $(EXES) $(OBJECTS) astut.o  *~

install: astut
	cp $< $$(dirname $$(which clang))
