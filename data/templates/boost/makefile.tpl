CXX=clang++ --std=c++17
DEPFLAGS = -MMD -MP
OBJS = tests.o # Please add your own .o files here
DEPS = $(OBJS:.o=.d)
TARGET = {target}_test

test: $(TARGET)
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^

clean:
	rm -rf $(TARGET) $(OBJS) *~

tests.o: $(TARGET).cpp
	$(CXX) $(DEPFLAGS) -c $< -o $@
	
compilation: tests.o

-include $(DEPS)
