CXX=clang++ --std=c++17
OBJS = tests.o main.o # Please add your own .o files here
TARGET = {target}_test

test: $(TARGET)
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^

clean:
	rm -rf $(TARGET) $(OBJS) *~

tests.o:
	$(CXX) $(TARGET).cpp -c $< -o tests.o

compilation: tests.o
