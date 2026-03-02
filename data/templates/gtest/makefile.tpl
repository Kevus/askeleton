CXX=clang++ --std=c++17
LIBS = -lgtest -lgtest_main -pthread
OBJS = {objectFiles}
TARGET = {target}_test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LIBS)

{sourceBuildRule}

tests.o: $(TARGET).cpp
	$(CXX) -c $< -o $@

main.o: main.cpp
	$(CXX) -c $< -o $@

clean:
	rm -rf $(TARGET) $(OBJS) *~
