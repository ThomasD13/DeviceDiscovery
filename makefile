SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
DEPS=$(SOURCES:.cpp=.d)
BINS= send receive

CFLAGS+=-MMD -g -Wall
CXXFLAGS+=-MMD -g -Wall

all: $(BINS)

send:
	g++ -o send $(CFLAGS) UdpTools.cpp UdpTools.h send.cpp

receive: 
	g++ -o receive $(CFLAGS) UdpTools.cpp UdpTools.h receive.cpp

.PHONY: clean

clean:
	$(RM) $(OBJECTS) $(DEPS) $(BINS)

-include $(DEPS)
