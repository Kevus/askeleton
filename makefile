CXX		:= clang++-15
RTTIFLAG	:= -fno-rtti
LLVMCXXFLAGS :=	\
	-g -I/usr/lib/llvm-15/include -fPIC -fvisibility-inlines-hidden	\
	-Werror=date-time -std=c++20 -Wall -W -Wno-unused-parameter -Wwrite-strings	\
	-Wcast-qual -Wno-missing-field-initializers -pedantic -Wno-long-long	\
	-Wno-uninitialized -Wdelete-non-virtual-dtor -Wno-comment -ffunction-sections	\
	-fdata-sections -O2 -DNDEBUG  -fno-exceptions -D_GNU_SOURCE	\
	-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -DFULL_DEBUG \
	-Iinclude/ -I.

CXXFLAGS	:= $(LLVMCXXFLAGS) $(RTTIFLAG) -fexceptions -gdwarf-4 -DFULL_DEBUG
LLVMLDFLAGS	:= $(shell llvm-config-15 --ldflags --system-libs --libs) $(LDFLAGS)

SOURCES =	\
	src/generators.cpp src/VariableInfo.cpp \
	auxiliary_functions.cpp ASKGen.cpp ASKMatchers.cpp \
	Generator/RandomValuesGenerator.cpp Generator/CustomGenerator.cpp \
	Generator/ConfigGenerator.cpp Generator/TestFrameworks.cpp askeleton.cpp

OBJECTS = $(SOURCES:.cpp=.o)
EXES = $(OBJECTS:.o=)

CLANGLIBS = \
	-lclangFrontend 	\
	-lclangSerialization	\
	-lclangDriver		\
	-lclangTooling		\
	-lclangParse		\
	-lclangSema		\
	-lclangAnalysis		\
	-lclangEdit		\
	-lclangAST		\
	-lclangASTMatchers	\
	-lclangLex		\
	-lclangBasic		\
	-lclangRewrite		\
	-lclangRewriteFrontend	\
	-lclangSupport


askeleton:	\
	src/generators.o src/VariableInfo.o \
	auxiliary_functions.o ASKGen.o ASKMatchers.o \
	Generator/RandomValuesGenerator.o Generator/CustomGenerator.o \
	Generator/ConfigGenerator.o Generator/TestFrameworks.o askeleton.o
	$(CXX) -o $@ $^ $(CLANGLIBS) $(LLVMLDFLAGS)

askeleton.o: auxiliary_functions.hpp ASKGen.hpp ASKMatchers.hpp Generator/RandomValuesGenerator.hpp Generator/CustomGenerator.hpp Generator/ConfigGenerator.hpp Generator/TestFrameworks.hpp	\
	src/generators.cpp src/VariableInfo.cpp auxiliary_functions.o ASKGen.o ASKMatchers.o Generator/RandomValuesGenerator.o Generator/CustomGenerator.o Generator/ConfigGenerator.o Generator/TestFrameworks.o

install: askeleton
	cp askeleton /usr/local/bin

clean:
	rm -f -r *.o Generator/*.o Generated/UT/* src/*.o Generated_LOG* askeleton
