CXX=clang++ --std=c++17 {extraCompileFlags}
CC=clang {extraCompileFlags}
DEPFLAGS = -MMD -MP
LIBS = -lgtest -lgtest_main -pthread
OBJS = {objectFiles}
DEPS = $(OBJS:.o=.d)
TARGET = {target}_test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LIBS)

{sourceBuildRule}

tests.o: $(TARGET).cpp
	$(CXX) $(DEPFLAGS) -c $< -o $@

main.o: main.cpp
	$(CXX) $(DEPFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET) $(OBJS) $(DEPS) *~

-include $(DEPS)
