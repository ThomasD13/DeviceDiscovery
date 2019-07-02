SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
DEPS=$(SOURCES:.cpp=.d)
BINS= send receive

CFLAGS+=-MMD -g
CXXFLAGS+=-MMD -g

all: $(BINS)

send:
	g++ $(CFLAGS) UdpTools.cpp UdpTools.h send.cpp

receive: 
	g++ $(CFLAGS) UdpTools.cpp UdpTools.h receive.cpp

.PHONY: clean

clean:
	$(RM) $(OBJECTS) $(DEPS) $(BINS)

-include $(DEPS)
