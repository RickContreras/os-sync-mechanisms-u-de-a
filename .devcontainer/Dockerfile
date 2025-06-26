# Alternative Dockerfile if you prefer custom image
FROM ubuntu:22.04

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install essential packages
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    gdb \
    valgrind \
    cmake \
    make \
    git \
    curl \
    wget \
    vim \
    nano \
    libc6-dbg \
    && rm -rf /var/lib/apt/lists/*

# Install Go
RUN curl -fsSL https://golang.org/dl/go1.21.5.linux-amd64.tar.gz | tar -xzC /usr/local
ENV PATH="/usr/local/go/bin:${PATH}"

# Create non-root user
RUN useradd -m -s /bin/bash vscode && \
    usermod -aG sudo vscode && \
    echo "vscode ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER vscode
WORKDIR /workspace

# Set up Go environment for user
RUN echo 'export PATH="/usr/local/go/bin:$PATH"' >> ~/.bashrc
RUN echo 'export GOPATH="$HOME/go"' >> ~/.bashrc