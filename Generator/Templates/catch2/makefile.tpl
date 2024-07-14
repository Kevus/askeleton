CXX=clang++ --std=c++14
OBJS = {cfgName}.o tests.o # Please add your own .o files here
TARGET = {cfgName}_test

test: $(TARGET)
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^

clean:
	rm -rf $(TARGET) $(OBJS) *~

{cfgName}.o: 
	$(CXX) {cppPath} -c $< -o {cfgName}.o

tests.o:
	$(CXX) $(TARGET).cpp -c $< -o tests.o

compilation: {cfgName}.o