#!/usr/bin/env bash

uv venv .venv

source .venv/bin/activate

uv sync

# Initialize PlatformIO to use local core directory
export PLATFORMIO_CORE_DIR=$(pwd)/.platformio

# Create symlink so PlatformIO's penv uses our venv
mkdir -p .platformio
ln -sf $(pwd)/.venv .platformio/penv

.venv/bin/pio run

.venv/bin/pio run -t compiledb

echo "\n\nrun source .venv/bin/activate\n\n"
