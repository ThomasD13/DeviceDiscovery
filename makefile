SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
DEPS=$(SOURCES:.cpp=.d)
BINS=$(SOURCES:.cpp=)

CFLAGS+=-MMD -g
CXXFLAGS+=-MMD -g

all: $(BINS)

.PHONY: clean

clean:
	$(RM) $(OBJECTS) $(DEPS) $(BINS)

-include $(DEPS)
