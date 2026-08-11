# Makefile for dynamixal-controller-poc
# Simple build system for Raspberry Pi Dynamixel controller

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
TARGET = dynamixal-controller
OUT_DIR = out
SRC_DIR = src
SOURCES = $(SRC_DIR)/main.cpp
LIBS = -ldxl_x64_cpp

# Include paths for Dynamixel SDK
INC_PATHS = -I/usr/local/include -I/usr/include

all: $(OUT_DIR)/$(TARGET)

$(OUT_DIR)/$(TARGET): $(SOURCES)
	@mkdir -p $(OUT_DIR)
	$(CXX) $(CXXFLAGS) $(INC_PATHS) $^ -o $@ $(LIBS)

run: $(OUT_DIR)/$(TARGET)
	./$<

build: $(OUT_DIR)/$(TARGET)

clean:
	rm -rf $(OUT_DIR)

.PHONY: all run build clean
