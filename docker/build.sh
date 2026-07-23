#!/bin/bash

# This script builds the development environment Docker image.

# --- Configuration ---
# The name for the Docker image we are building.
IMAGE_NAME="openrb150-dev-env"

# Get the absolute path to the project root (the parent directory of this script's location)
# This is used as the build context so that .dockerignore works correctly.
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Project root (build context): ${PROJECT_ROOT}"
echo "Dockerfile path: ${PROJECT_ROOT}/docker/Dockerfile"

# --- Build the Docker Image ---
echo "Building Docker image '${IMAGE_NAME}'..."

# We use the project root as the build context.
# -t: Tags the image with a name (e.g., "openrb150-dev-env").
# -f: Specifies the path to the Dockerfile.
# The final argument "." sets the build context to the project root.
docker build -t "${IMAGE_NAME}" -f "${PROJECT_ROOT}/docker/Dockerfile" "${PROJECT_ROOT}"

# Check if the build was successful
if [ $? -ne 0 ]; then
    echo "Docker build failed. Aborting."
    exit 1
fi

echo ""
echo "Docker image '${IMAGE_NAME}' built successfully."
echo "You can now start the development environment by running: docker/start-dev.sh"
