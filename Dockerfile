# Multi-stage Dockerfile for ASTERIX radar integration suite
# Includes C++ decoder, Python module, and radar integration examples
#
# Build: docker build -t asterix:latest .
# Run:   docker run --rm asterix asterix --version
#
# Image size target: <500 MB
# Security: Non-root user, minimal attack surface

# Stage 1: Build C++ ASTERIX decoder and Python extension
FROM ubuntu:22.04 AS builder

# Set non-interactive to avoid timezone prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    g++ \
    gcc \
    libexpat1-dev \
    python3 \
    python3-dev \
    python3-pip \
    python3-setuptools \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Create build directory
WORKDIR /build

# Copy source files (ORDER MATTERS - copy deps first for better caching)
COPY VERSION /build/
COPY CMakeLists.txt /build/
COPY setup.py /build/
COPY pyproject.toml /build/
COPY requirements.txt /build/
COPY LICENSE /build/

# Copy all documentation files required by CMake install
COPY *.md /build/

# Copy source code
COPY src/ /build/src/
COPY asterix/ /build/asterix/

# Build C++ ASTERIX decoder with CMake
RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    && cmake --build build --parallel $(nproc) \
    && cmake --install build

# Build Python module (uses CMake build artifacts)
RUN pip3 install --no-cache-dir --prefix=/usr/local .

# Stage 2: Runtime image (smaller, production-ready)
FROM ubuntu:22.04

# Metadata labels (OCI standard)
LABEL org.opencontainers.image.title="ASTERIX Radar Integration Suite"
LABEL org.opencontainers.image.description="ASTERIX decoder/encoder with radar simulation integration (CAT001, 019, 020, 021, 034, 048, 062)"
LABEL org.opencontainers.image.version="2.8.10"
LABEL org.opencontainers.image.authors="Damir Salantic <damir.salantic@gmail.com>"
LABEL org.opencontainers.image.source="https://github.com/montge/asterix"
LABEL org.opencontainers.image.licenses="GPL-3.0-or-later"
LABEL org.opencontainers.image.documentation="https://github.com/montge/asterix/blob/master/README.md"

# Set non-interactive
ENV DEBIAN_FRONTEND=noninteractive

# Install runtime dependencies only (minimal footprint)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libexpat1 \
    python3 \
    python3-pip \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user for security
RUN groupadd -r asterix && useradd -r -g asterix -m -s /bin/bash asterix

# Copy built artifacts from builder
COPY --from=builder /usr/local/bin/asterix /usr/local/bin/asterix
COPY --from=builder /usr/local/lib /usr/local/lib
COPY --from=builder /usr/local/share/asterix /usr/local/share/asterix

# Copy Python module (pip installs to /usr/local/local/lib due to --prefix=/usr/local)
COPY --from=builder /usr/local/local/lib/python3.10/dist-packages/ /usr/local/lib/python3.10/dist-packages/

# Symlink config directory to where Python module expects it
RUN ln -s /usr/local/share/asterix/config /usr/local/lib/python3.10/dist-packages/asterix/config

# Configure dynamic linker
RUN ldconfig

# Install Python dependencies for radar_integration
# Keep base image minimal - install matplotlib, pymavlink, jsbsim as optional extras
RUN pip3 install --no-cache-dir \
    lxml>=4.9.0

# Install numpy for radar integration (required by mock_radar)
RUN pip3 install --no-cache-dir numpy

# Copy radar integration examples and utilities
COPY examples/radar_integration /home/asterix/radar_integration
COPY .local/integration/mock_radar.py /usr/local/lib/python3.10/dist-packages/asterix/radar_integration/__init__.py
COPY .local/integration/asterix_encoder /usr/local/lib/python3.10/dist-packages/asterix/radar_integration/encoder

# Create __init__.py stub for encoder module and fix permissions
RUN echo "from .cat048 import encode_cat048" > /usr/local/lib/python3.10/dist-packages/asterix/radar_integration/encoder/__init__.py \
    && chmod -R 755 /usr/local/lib/python3.10/dist-packages/asterix/radar_integration

# Create data and config directories
RUN mkdir -p /home/asterix/data /home/asterix/config

# Set ownership to asterix user
RUN chown -R asterix:asterix /home/asterix

# Switch to non-root user
USER asterix
WORKDIR /home/asterix

# Add lib directory to PYTHONPATH for imports
ENV PYTHONPATH="/home/asterix/lib:${PYTHONPATH}"

# Health check - verify asterix binary works
HEALTHCHECK --interval=30s --timeout=5s --start-period=5s --retries=3 \
    CMD asterix --version || exit 1

# Default command: show help
CMD ["asterix", "--help"]
