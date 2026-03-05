CXX ?= clang++
CC ?= clang
CPPFLAGS += {extraCompileFlags} $(EXTRA_CPPFLAGS)
CXXFLAGS += --std=c++17 $(EXTRA_CXXFLAGS)
CFLAGS += $(EXTRA_CFLAGS)
DEPFLAGS = -MMD -MP
LIBS ?= -lgtest -lgtest_main -pthread
LDFLAGS += $(EXTRA_LDFLAGS)
EXTRA_LIBS ?=
OBJS = {objectFiles}
DEPS = $(OBJS:.o=.d)
TARGET = {target}_test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS) $(EXTRA_LIBS)

{sourceBuildRule}

tests.o: $(TARGET).cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

main.o: main.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET) $(OBJS) $(DEPS) *~

-include $(DEPS)
