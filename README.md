# Dynamixal Controller POC

A proof of concept program that provides a command-line interface to control the angle of Dynamixel servo motors via a USB adapter.

## Prerequisites

- [Nix](https://nixos.org/) with flakes enabled
- Dynamixel servos connected via FTDI USB adapter

## Build

```bash
nix build
```

The binary is available at `result/bin/dynamixal-controller`.

## Run

With defaults (`/dev/ttyUSB0`, baud 57600, servo ID 1):

```bash
nix run .
```

With explicit options:

```bash
nix run . -- --device /dev/ttyUSB0 --baud 57600 --id 1
```

Run `nix run . -- --help` to see all options.

## Dev Shell

```bash
nix develop
```

Provides gcc, cmake, ninja, and the Dynamixel SDK on the path.

## FTDI Latency Timer

The FTDI USB adapter defaults to a 16ms latency timer, which causes noticeable lag in servo communication. A udev rule is included to set it to 1ms automatically when the device is plugged in:

```bash
sudo cp 40-dynamixel-ftdi.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## Usage

Once running, the program will:
1. Open the serial port (`/dev/ttyUSB0` by default)
2. Set the baud rate (57600 by default)
3. Ping the servo to verify communication (auto-detects Protocol 1.0 or 2.0)
4. Enable torque on the servo

Enter target positions (0-4095) to move the servo. Enter `-1` to exit.

## Checks

```bash
nix flake check
```

Runs a smoke test that builds the binary and verifies `--help` exits successfully.
```
