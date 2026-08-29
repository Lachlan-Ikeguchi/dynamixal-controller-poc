OUT_DIR = out
TARGET = dynamixal-controller

build:
	cmake -S . -B $(OUT_DIR) -G "Ninja"
	cmake --build $(OUT_DIR)

run: build
	./$(OUT_DIR)/$(TARGET)

clean:
	rm -rf $(OUT_DIR)

.PHONY: build run clean
