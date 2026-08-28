# Makefile for dynamixal-controller-poc
# Simple build system for Raspberry Pi Dynamixel controller

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
OUT_DIR = out
SRC_DIR = src
LIBS = -ldxl_x64_cpp

# Include paths for Dynamixel SDK and local headers
INC_PATHS = -I/usr/local/include -I/usr/include -Iinclude

TARGET = dynamixal-controller
TARGET_TEST = test-main

SOURCES = $(SRC_DIR)/main.cpp
SOURCES_TEST = $(SRC_DIR)/test_main.cpp $(SRC_DIR)/dynamixel_controller.cpp

all: $(OUT_DIR)/$(TARGET)

$(OUT_DIR)/$(TARGET): $(SOURCES)
	@mkdir -p $(OUT_DIR)
	$(CXX) $(CXXFLAGS) $(INC_PATHS) $^ -o $@ $(LIBS)

$(OUT_DIR)/$(TARGET_TEST): $(SOURCES_TEST)
	@mkdir -p $(OUT_DIR)
	$(CXX) $(CXXFLAGS) $(INC_PATHS) $^ -o $@ $(LIBS)

run: $(OUT_DIR)/$(TARGET)
	./$<

run-test: $(OUT_DIR)/$(TARGET_TEST)
	./$<

build: $(OUT_DIR)/$(TARGET)

test: $(OUT_DIR)/$(TARGET_TEST)

clean:
	rm -rf $(OUT_DIR)

.PHONY: all run build clean test run-test
