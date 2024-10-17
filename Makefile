CXX = clang++-15
INCLUDES = -Iinclude/ -I$(shell llvm-config-15 --includedir)
CXXFLAGS = -std=c++20 -Wall -Wextra -Wcast-qual -Wwrite-strings -Wno-unused-parameter \
           -Wdelete-non-virtual-dtor -fPIC -ffunction-sections -fdata-sections #-MMD -MP
CLANGLIBS = $(shell llvm-config-15 --ldflags --system-libs --libs) \
			-lclangFrontend -lclangSerialization -lclangDriver -lclangTooling \
			-lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST \
			-lclangASTMatchers -lclangLex -lclangBasic -lclangRewrite \
			-lclangRewriteFrontend -lclangSupport
DEBUG_FLAGS = -DFULL_DEBUG # -g ON DEBUG
OPTIMIZATION_FLAGS = -O0 # -O2 ON RELEASE

SRCS = $(wildcard src/*.cpp) $(wildcard src/framework/*.cpp)
OBJS = $(SRCS:.cpp=.o)

TARGET = askeleton

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