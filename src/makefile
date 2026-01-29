# Makefile for Project 1
# Name: Jaden Zhou
# Purpose: Build OpenCV programs for CS5330

# Compiler
CXX = clang++

# Directories
BINDIR = ../bin
INCDIR = ../include

# Libraries
LDLIBS = $(shell pkg-config --libs opencv4)

# ONNX Runtime (Homebrew)
ONNX_INC = -I/opt/homebrew/opt/onnxruntime/include
ONNX_LIB = -L/opt/homebrew/opt/onnxruntime/lib -lonnxruntime

# Flags
CXXFLAGS = -std=c++17 -I$(INCDIR) $(shell pkg-config --cflags opencv4) $(ONNX_INC)


.PHONY: all clean


all: img vid img-effects


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


img: imgDisplay.o
	mkdir -p $(BINDIR)
	$(CXX) $^ -o $(BINDIR)/$@ $(LDLIBS)


vid: vidDisplay.o filters.o faceDetect.o
	mkdir -p $(BINDIR)
	$(CXX) $^ -o $(BINDIR)/$@ $(LDLIBS) $(ONNX_LIB)


time: timeBlur.o filters.o
	mkdir -p $(BINDIR)
	$(CXX) $^ -o $(BINDIR)/$@ $(LDLIBS)

img-effects: imgEffects.o filters.o faceDetect.o
	mkdir -p $(BINDIR)
	$(CXX) $^ -o $(BINDIR)/$@ $(LDLIBS) $(ONNX_LIB)

da2-img: da2-example.o
	mkdir -p $(BINDIR)
	$(CXX) $^ -o $(BINDIR)/$@ $(LDLIBS) $(ONNX_LIB)

da2-vid: da2-video.o
	mkdir -p $(BINDIR)
	$(CXX) $^ -o $(BINDIR)/$@ $(LDLIBS) $(ONNX_LIB)


clean:
	rm -f *.o *~
	rm -f $(BINDIR)/img $(BINDIR)/vid $(BINDIR)/time