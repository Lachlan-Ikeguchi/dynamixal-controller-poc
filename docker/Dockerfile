FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV PIO_HOME_DIR=/workspace/.pio

RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    python3-venv \
    git \
    curl \
    ca-certificates \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    make \
    udev \
    usbutils \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install --upgrade pip && \
    pip install --no-cache-dir -U platformio

RUN curl -fsSL \
    https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules \
    -o /etc/udev/rules.d/99-platformio-udev.rules

RUN mkdir -p /workspace
WORKDIR /workspace

CMD ["/bin/bash"]
