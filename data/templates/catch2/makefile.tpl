DEFAULT_CLANGXX := $(shell if command -v clang++-18 >/dev/null 2>&1; then printf '%s' clang++-18; elif command -v clang++ >/dev/null 2>&1; then printf '%s' clang++; else printf '%s' clang++; fi)
DEFAULT_CLANG := $(shell if command -v clang-18 >/dev/null 2>&1; then printf '%s' clang-18; elif command -v clang >/dev/null 2>&1; then printf '%s' clang; else printf '%s' clang; fi)
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
LIBS ?= $(shell if ldconfig -p 2>/dev/null | grep -q 'libCatch2Main'; then printf '%s' '-lCatch2Main -lCatch2 -pthread'; elif [ -e /usr/lib/libCatch2Main.a ] || [ -e /usr/lib/x86_64-linux-gnu/libCatch2Main.a ] || [ -e /usr/local/lib/libCatch2Main.a ] || [ -e /usr/local/lib/libCatch2Main.so ] || [ -e /usr/lib/x86_64-linux-gnu/libCatch2Main.so ]; then printf '%s' '-lCatch2Main -lCatch2 -pthread'; fi)
LDFLAGS += $(EXTRA_LDFLAGS)
EXTRA_LIBS ?=
OBJS = {sourceObjectFiles} tests.o $(if $(LIBS),,main.o)
DEPS = $(OBJS:.o=.d)
TARGET = {target}_test

all: $(TARGET)
test: $(TARGET)
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@.tmp $^ $(LIBS) $(EXTRA_LIBS)
	mv $@.tmp $@

{sourceBuildRule}

clean:
	rm -rf $(TARGET) $(TARGET).tmp $(OBJS) $(DEPS) *~

tests.o: $(TARGET).cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

main.o: main.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

compilation: tests.o

-include $(DEPS)
