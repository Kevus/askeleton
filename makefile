CXX		:= clang++
RTTIFLAG	:= -fno-rtti
LLVMCXXFLAGS :=	\
	-g -I/usr/lib/llvm-15/include -std=c++0x -fPIC -fvisibility-inlines-hidden	\
	-Werror=date-time -std=c++14 -Wall -W -Wno-unused-parameter -Wwrite-strings	\
	-Wcast-qual -Wno-missing-field-initializers -pedantic -Wno-long-long	\
	-Wno-uninitialized -Wdelete-non-virtual-dtor -Wno-comment -ffunction-sections	\
	-fdata-sections -O2 -DNDEBUG  -fno-exceptions -D_GNU_SOURCE	\
	-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS

CXXFLAGS	:= $(LLVMCXXFLAGS) $(RTTIFLAG) -fexceptions
LLVMLDFLAGS	:= $(shell llvm-config --ldflags --system-libs --libs) $(LDFLAGS)

DIRC = Containers/
DIRGEN = Generator/

SOURCES =	\
	auxiliary_functions.cpp ASTUTGen.cpp ASTUTMatchers.cpp \
	Generator/RandomValuesGenerator.cpp Generator/CustomGenerator.cpp \
	Generator/ConfigGenerator.cpp Generator/TestFrameworks.cpp astut.cpp

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
	auxiliary_functions.o ASTUTGen.o ASTUTMatchers.o \
	Generator/RandomValuesGenerator.o Generator/CustomGenerator.o \
	Generator/ConfigGenerator.o Generator/TestFrameworks.o astut.o
	$(CXX) -o $@ $^ $(CLANGLIBS) $(LLVMLDFLAGS)

askeleton.o: auxiliary_functions.hpp ASTUTGen.hpp ASTUTMatchers.hpp Generator/RandomValuesGenerator.hpp Generator/CustomGenerator.hpp Generator/ConfigGenerator.hpp Generator/TestFrameworks.hpp	\
	auxiliary_functions.o ASTUTGen.o ASTUTMatchers.o Generator/RandomValuesGenerator.o Generator/CustomGenerator.o Generator/ConfigGenerator.o Generator/TestFrameworks.o

clean:
	rm -f -r *.o Generator/*.o Generated/UT/* Generated_LOG* askeleton
