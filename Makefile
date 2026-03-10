DEFAULT_CLANGXX := $(shell if command -v clang++ >/dev/null 2>&1; then printf '%s' clang++; elif command -v clang++-18 >/dev/null 2>&1; then printf '%s' clang++-18; else printf '%s' clang++; fi)
DEFAULT_LLVM_CONFIG := $(shell if command -v llvm-config >/dev/null 2>&1; then printf '%s' llvm-config; elif command -v llvm-config-18 >/dev/null 2>&1; then printf '%s' llvm-config-18; else printf '%s' llvm-config; fi)

ifeq ($(origin CXX), default)
CXX := $(DEFAULT_CLANGXX)
endif
ifeq ($(origin CXX), undefined)
CXX := $(DEFAULT_CLANGXX)
endif
LLVM_SUFFIX := $(patsubst clang++%,%,$(notdir $(CXX)))
ifneq ($(LLVM_SUFFIX),)
LLVM_CONFIG_CANDIDATE := llvm-config$(LLVM_SUFFIX)
ifneq ($(shell command -v $(LLVM_CONFIG_CANDIDATE) 2>/dev/null),)
ifeq ($(origin LLVM_CONFIG), undefined)
LLVM_CONFIG := $(LLVM_CONFIG_CANDIDATE)
endif
else
ifeq ($(origin LLVM_CONFIG), undefined)
LLVM_CONFIG := $(DEFAULT_LLVM_CONFIG)
endif
endif
else
ifeq ($(origin LLVM_CONFIG), undefined)
LLVM_CONFIG := $(DEFAULT_LLVM_CONFIG)
endif
endif
INCLUDES = -Iinclude/ -I$(shell $(LLVM_CONFIG) --includedir)
CXXFLAGS = -std=c++20 -Wall -Wextra -Wcast-qual -Wwrite-strings -Wno-unused-parameter \
           -Wdelete-non-virtual-dtor -fPIC -ffunction-sections -fdata-sections
DEPFLAGS = -MMD -MP
CLANGLIBS = $(shell $(LLVM_CONFIG) --ldflags --system-libs --libs) \
			-lclangFrontend -lclangAPINotes -lclangSerialization -lclangDriver -lclangTooling \
			-lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST \
			-lclangASTMatchers -lclangLex -lclangBasic -lclangRewrite \
			-lclangRewriteFrontend -lclangSupport
DEBUG_FLAGS = -DFULL_DEBUG -g # -g ON DEBUG
OPTIMIZATION_FLAGS = -O0 # -O2 ON RELEASE

SRCS = $(wildcard src/*.cpp) $(wildcard src/framework/*.cpp) $(wildcard src/utils/*.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(OBJS:.o=.d)

TARGET = askeleton

ALLOW_NON_CLANG ?= 0
ifeq ($(ALLOW_NON_CLANG),1)
$(warning ALLOW_NON_CLANG=1: proceeding with CXX=$(CXX), results may differ)
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(CLANGLIBS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(INCLUDES) $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS) -c $< -o $@

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin

clean:
	rm -f ${OBJS} $(DEPS) $(TARGET)

-include $(DEPS)

.PHONY: clean

# Generic clang check (accepts clang++, clang++-18, etc.)
ifneq ($(ALLOW_NON_CLANG),1)
CLANG_DETECTED := $(shell $(CXX) --version 2>/dev/null | head -n 1 | grep -Eqi "clang" && echo yes || echo no)
ifneq ($(CLANG_DETECTED),yes)
$(error CXX must be clang. Set CXX=clang++ or ALLOW_NON_CLANG=1)
endif
endif
