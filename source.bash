#!/usr/bin/env bash

# Source the virtual environment
if [ -f .venv/bin/activate ]; then
    source .venv/bin/activate
else
    echo "Error: .venv not found. Please run setup.bash first."
    return 1
fi

# Export PlatformIO environment variables
export PLATFORMIO_CORE_DIR=$(pwd)/.platformio

# Create symlink for PlatformIO's penv if it doesn't exist
if [ ! -L .platformio/penv ] && [ -d .venv ]; then
    mkdir -p .platformio
    ln -sf $(pwd)/.venv .platformio/penv
fi

echo "Environment activated. PlatformIO will use local core directory and venv."
