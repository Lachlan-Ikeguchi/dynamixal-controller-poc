#!/usr/bin/env bash
set -euo pipefail

# --- Configuration ---
IMAGE_NAME="${IMAGE_NAME:-openrb150-dev-env}"
CONTAINER_NAME="${CONTAINER_NAME:-openrb150-dev}"

# Project root = repo root (docker/ 的上一層)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# If you are on ARM64 host (Jetson/RPi), you MUST run amd64 container.
# You can override by: PIO_PLATFORM=linux/arm64 ./docker/run.sh
PIO_PLATFORM="${PIO_PLATFORM:-linux/amd64}"

# Try to detect serial device (Linux)
SERIAL_DEV="${SERIAL_DEV:-}"
if [[ -z "${SERIAL_DEV}" ]]; then
  for d in /dev/ttyACM0 /dev/ttyACM1 /dev/ttyUSB0 /dev/ttyUSB1; do
    if [[ -e "$d" ]]; then
      SERIAL_DEV="$d"
      break
    fi
  done
fi

# --- Check if Image Exists ---
if [[ -z "$(docker images -q "${IMAGE_NAME}" 2>/dev/null)" ]]; then
  echo "Error: Docker image '${IMAGE_NAME}' not found."
  echo "Please build it first, e.g.: docker build -t ${IMAGE_NAME} -f docker/Dockerfile ."
  exit 1
fi

echo "Starting container '${CONTAINER_NAME}'..."
echo "Project mounted at /workspace"
echo "Container platform: ${PIO_PLATFORM}"
if [[ -n "${SERIAL_DEV}" ]]; then
  echo "Serial device: ${SERIAL_DEV}"
else
  echo "Serial device: (not detected)  -> you can set SERIAL_DEV=/dev/ttyACM0"
fi
echo
echo "Inside container, build with:   pio run -v"
echo "Upload with:                   pio run -t upload -v"
echo "Exit with Ctrl+D or 'exit'"
echo

# --- Run ---
RUN_ARGS=( -it --rm
  --name "${CONTAINER_NAME}"
  --platform "${PIO_PLATFORM}"
  -v "${PROJECT_ROOT}:/workspace"
  -w /workspace
)

# Prefer --device if detected, otherwise fallback to --privileged for simplicity
if [[ -n "${SERIAL_DEV}" ]]; then
  RUN_ARGS+=( --device "${SERIAL_DEV}:${SERIAL_DEV}" )
else
  RUN_ARGS+=( --privileged )
fi

docker run "${RUN_ARGS[@]}" "${IMAGE_NAME}" /bin/bash
echo "Container stopped."
