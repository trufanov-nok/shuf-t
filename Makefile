CXX = $(shell which g++)

ifneq ($(CXX),)
  
else
  CXX = clang++
  $(warning Using clang++)
endif

ifeq ($(CXX),)
  $(warning No compiler found)
  exit 1
endif

CFLAGS=-c -Wall -O3
LDFLAGS=
SOURCES=utils.cpp main.cpp shuf-t.cpp io_buf.cc
OBJECTS=$(SOURCES:=.o)
EXECUTABLE=shuf-t

all: $(SOURCES) $(EXECUTABLE)
	 
$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@
%.cc.o: %.cc
	$(CXX) $(CFLAGS) $< -o $@
%.cpp.o: %.cpp
	$(CXX) $(CFLAGS) $< -o $@    
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
install:
	cp $(EXECUTABLE) /usr/bin/$(EXECUTABLE)