CXX=clang++ --std=c++17
DEPFLAGS = -MMD -MP
CATCH2_LIBS = $(shell if ldconfig -p 2>/dev/null | grep -q 'libCatch2Main'; then printf '%s' '-lCatch2Main -lCatch2 -pthread'; elif [ -e /usr/lib/libCatch2Main.a ] || [ -e /usr/lib/x86_64-linux-gnu/libCatch2Main.a ] || [ -e /usr/local/lib/libCatch2Main.a ] || [ -e /usr/local/lib/libCatch2Main.so ] || [ -e /usr/lib/x86_64-linux-gnu/libCatch2Main.so ]; then printf '%s' '-lCatch2Main -lCatch2 -pthread'; fi)
OBJS = tests.o $(if $(CATCH2_LIBS),,main.o) # Please add your own .o files here
DEPS = $(OBJS:.o=.d)
TARGET = {target}_test

test: $(TARGET)
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CATCH2_LIBS)

clean:
	rm -rf $(TARGET) $(OBJS) *~

tests.o: $(TARGET).cpp
	$(CXX) $(DEPFLAGS) -c $< -o $@

main.o: main.cpp
	$(CXX) $(DEPFLAGS) -c $< -o $@

compilation: tests.o

-include $(DEPS)
