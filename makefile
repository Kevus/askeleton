CXX		:= clang++
RTTIFLAG	:= -fno-rtti
LLVMCXXFLAGS :=	\
	-I/usr/lib/llvm-6.0/include -std=c++0x -fPIC -fvisibility-inlines-hidden	\
	-Werror=date-time -std=c++11 -Wall -W -Wno-unused-parameter -Wwrite-strings	\
	-Wcast-qual -Wno-missing-field-initializers -pedantic -Wno-long-long	\
	-Wno-uninitialized -Wdelete-non-virtual-dtor -Wno-comment -ffunction-sections	\
	-fdata-sections -O2 -DNDEBUG  -fno-exceptions -D_GNU_SOURCE	\
	-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS

CXXFLAGS	:= $(LLVMCXXFLAGS) $(RTTIFLAG) -fexceptions
LLVMLDFLAGS	:= $(shell llvm-config --ldflags --system-libs --libs) $(LDFLAGS)

DIRC = Containers/
DIRGEN = Generator/

SOURCES =	\
	RVisitor.cpp $(DIRC)FunctionDeclParameterCont.cpp $(DIRC)FunctionDeclCont.cpp	\
	$(DIRC)CXXMethodDeclCont.cpp $(DIRC)CXXRecordDeclAttributeCont.cpp	\
	$(DIRC)CXXRecordDeclCont.cpp $(DIRC)EnumDeclCont.cpp $(DIRC)ASTUTArchive.cpp	\
	$(DIRGEN)Boost/ConfigGenerator.cpp $(DIRGEN)Boost/BoostTestGenerator.cpp

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
	-lclangRewriteFrontend


RVisitor:	\
	$(DIRC)FunctionDeclParameterCont.o $(DIRC)FunctionDeclCont.o $(DIRC)CXXMethodDeclCont.o	\
	$(DIRC)CXXRecordDeclAttributeCont.o $(DIRC)CXXRecordDeclCont.o $(DIRC)EnumDeclCont.o \
	$(DIRC)ASTUTArchive.o $(DIRGEN)Boost/ConfigGenerator.o $(DIRGEN)Boost/BoostTestGenerator.o  RVisitor.o 
	$(CXX) -o $@ $^ $(CLANGLIBS) $(LLVMLDFLAGS)

RVisitor.o: $(DIRC)FunctionDeclParameterCont.hpp $(DIRC)FunctionDeclCont.hpp	\
	$(DIRC)CXXMethodDeclCont.hpp $(DIRC)CXXRecordDeclAttributeCont.hpp	\
	$(DIRC)CXXRecordDeclCont.hpp $(DIRC)EnumDeclCont.hpp $(DIRC)ASTUTArchive.hpp	\
	$(DIRGEN)Boost/ConfigGenerator.hpp $(DIRGEN)Boost/BoostTestGenerator.hpp	\
	$(DIRC)FunctionDeclParameterCont.o $(DIRC)FunctionDeclCont.o $(DIRC)CXXMethodDeclCont.o	\
	$(DIRC)CXXRecordDeclAttributeCont.o $(DIRC)CXXRecordDeclCont.o $(DIRC)EnumDeclCont.o	\
	$(DIRC)ASTUTArchive.o $(DIRGEN)Boost/ConfigGenerator.o $(DIRGEN)Boost/BoostTestGenerator.o

clean:
	rm -f *.o $(DIRC)*.o  RVisitor
	rm -f -r GeneratedBACKUP
	mv Generated GeneratedBACKUP
