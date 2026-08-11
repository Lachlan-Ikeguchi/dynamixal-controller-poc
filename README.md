# Dynamixal Controller POC

A proof of concept program that runs on a Raspberry Pi and provides an interface through WiFi to control the angle of Dynamixel servo arm motors.

## Prerequisites

- Raspberry Pi (or other Linux SBC)
- g++ compiler (C++17 support)
- Dynamixel SDK C++ library (`libdxl_x64_cpp`)
- Dynamixel servos connected via USB adapter

### Install Dynamixel SDK

The project requires the ROBOTIS Dynamixel SDK. Install it from source:

```bash
# Clone the SDK
git clone https://github.com/ROBOTIS-GIT/DynamixelSDK.git
cd DynamixelSDK/c++/build/linux64
make
sudo make install
```

This installs the library to `/usr/local/lib` and headers to `/usr/local/include`.

## Build

```bash
make
```

This compiles the source and creates the executable at `out/dynamixal-controller`.

## Run

```bash
make run
```

Or manually:

```bash
./out/dynamixal-controller
```

## Usage

Once running, the program will:
1. Attempt to open the serial port (`/dev/ttyUSB0` by default)
2. Set the baud rate to 57600
3. Ping the servo (ID 1) to verify communication
4. Enable torque on the servo

You can then enter target positions (0-4095) to move the servo. Enter `-1` to exit.

## Clean

Remove build artifacts:

```bash
make clean
```

## Project Structure

```
.
├── include/     # Header files
├── lib/         # Library source
├── out/         # Build output (ignored by git)
├── src/         # Application source
│   └── main.cpp
├── test/        # Tests
├── Makefile     # Build configuration
└── README.md
```
