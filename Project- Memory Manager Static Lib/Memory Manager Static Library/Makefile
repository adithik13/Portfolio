CXX = g++
CXXFLAGS = -Wall -Wextra -I.
LIBRARY = libMemoryManager.a
SRCS = MemoryManager.cpp
OBJS = $(SRCS:.cpp=.o)
all: $(LIBRARY)
$(LIBRARY): $(OBJS)
	$(AR) rcs $@ $^
%.o: %.cpp MemoryManager.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f $(OBJS) $(LIBRARY)

.PHONY: all clean
