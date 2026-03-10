DEFAULT_CLANGXX := $(shell if command -v clang++ >/dev/null 2>&1; then printf '%s' clang++; elif command -v clang++-18 >/dev/null 2>&1; then printf '%s' clang++-18; else printf '%s' clang++; fi)
DEFAULT_CLANG := $(shell if command -v clang >/dev/null 2>&1; then printf '%s' clang; elif command -v clang-18 >/dev/null 2>&1; then printf '%s' clang-18; else printf '%s' clang; fi)
ifeq ($(origin CXX), default)
CXX := $(DEFAULT_CLANGXX)
endif
ifeq ($(origin CXX), undefined)
CXX := $(DEFAULT_CLANGXX)
endif
ifeq ($(origin CC), default)
CC := $(DEFAULT_CLANG)
endif
ifeq ($(origin CC), undefined)
CC := $(DEFAULT_CLANG)
endif
CPPFLAGS += {extraCompileFlags} $(EXTRA_CPPFLAGS)
CXXFLAGS += --std=c++17 $(EXTRA_CXXFLAGS)
CFLAGS += $(EXTRA_CFLAGS)
DEPFLAGS = -MMD -MP
LDFLAGS += $(EXTRA_LDFLAGS)
LIBS ?=
EXTRA_LIBS ?=
OBJS = {sourceObjectFiles} tests.o
DEPS = $(OBJS:.o=.d)
TARGET = {target}_test

all: $(TARGET)
test: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS) $(EXTRA_LIBS)

{sourceBuildRule}

tests.o: $(TARGET).cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET) $(OBJS) $(DEPS) *~

compilation: tests.o

-include $(DEPS)
