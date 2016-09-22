SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
DEPS    := $(SOURCES:.cpp=.d)
CXXFLAGS += -std=c++14 -Wall -Wextra -pedantic -Werror -I. -MMD -MP -g

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: aina

aina: $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f aina
	rm -f $(OBJECTS)
	rm -f $(DEPS)

-include $(DEPS)
