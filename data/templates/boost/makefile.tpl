CXX=clang++ --std=c++14
OBJS = {target}.o tests.o # Please add your own .o files here
TARGET = {target}_test

test: $(TARGET)
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^

clean:
	rm -rf $(TARGET) $(OBJS) *~

{target}.o: 
	$(CXX) {cppPath} -c $< -o {target}.o

tests.o:
	$(CXX) $(TARGET).cpp -c $< -o tests.o

compilation: {target}.o

