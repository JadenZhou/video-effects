# Makefile for Project 1
# Name: Jaden Zhou
# Purpose: Build OpenCV programs for CS5330

# Compiler
CXX = clang++

# Directories
BINDIR = ../bin
INCDIR = ../include

# Flags
CXXFLAGS = -std=c++17 -I$(INCDIR) $(shell pkg-config --cflags opencv4)

# Libraries
LDLIBS = $(shell pkg-config --libs opencv4)


.PHONY: all clean


all: img vid


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


img: imgDisplay.o
	mkdir -p $(BINDIR)
	$(CXX) $^ -o $(BINDIR)/$@ $(LDLIBS)


vid: vidDisplay.o filters.o
	mkdir -p $(BINDIR)
	$(CXX) $^ -o $(BINDIR)/$@ $(LDLIBS)

time: timeBlur.o filters.o
	mkdir -p $(BINDIR)
	$(CXX) $^ -o $(BINDIR)/$@ $(LDLIBS)


clean:
	rm -f *.o *~
	rm -f $(BINDIR)/img $(BINDIR)/vid