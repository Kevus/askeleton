CXX ?= clang++-18
LLVM_CONFIG ?= llvm-config-18
INCLUDES = -Iinclude/ -I$(shell $(LLVM_CONFIG) --includedir)
CXXFLAGS = -std=c++20 -Wall -Wextra -Wcast-qual -Wwrite-strings -Wno-unused-parameter \
           -Wdelete-non-virtual-dtor -fPIC -ffunction-sections -fdata-sections #-MMD -MP
CLANGLIBS = $(shell $(LLVM_CONFIG) --ldflags --system-libs --libs) \
			-lclangFrontend -lclangAPINotes -lclangSerialization -lclangDriver -lclangTooling \
			-lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST \
			-lclangASTMatchers -lclangLex -lclangBasic -lclangRewrite \
			-lclangRewriteFrontend -lclangSupport
DEBUG_FLAGS = -DFULL_DEBUG -g # -g ON DEBUG
OPTIMIZATION_FLAGS = -O0 # -O2 ON RELEASE

SRCS = $(wildcard src/*.cpp) $(wildcard src/framework/*.cpp) $(wildcard src/utils/*.cpp)
OBJS = $(SRCS:.cpp=.o)

TARGET = askeleton

ALLOW_NON_CLANG ?= 0
ifneq ($(ALLOW_NON_CLANG),1)
ifneq ($(findstring clang,$(CXX)),clang)
$(error CXX must be clang (e.g., clang++-18). Set CXX=clang++-18 or ALLOW_NON_CLANG=1 to override.)
endif
else
$(warning ALLOW_NON_CLANG=1: proceeding with CXX=$(CXX), results may differ)
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(CLANGLIBS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS) -c $< -o $@

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin

clean:
	rm -f ${OBJS} $(TARGET)

.PHONY: clean
