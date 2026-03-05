CXX ?= clang++
CC ?= clang
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
