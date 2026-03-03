CXX=clang++ --std=c++17
LIBS = -lCatch2Main -lCatch2 -pthread
OBJS = tests.o # Please add your own .o files here
TARGET = {target}_test

test: $(TARGET)
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LIBS)

clean:
	rm -rf $(TARGET) $(OBJS) *~

tests.o:
	$(CXX) $(TARGET).cpp -c $< -o tests.o

compilation: tests.o
