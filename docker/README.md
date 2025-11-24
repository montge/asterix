# Docker Containerization for ASTERIX Radar Integration Suite

This directory contains Docker configuration for running the complete ASTERIX radar integration suite in a containerized environment.

## Quick Start

```bash
# Build the image
docker build -t asterix:latest .

# Run ASTERIX decoder
docker run --rm asterix asterix --version

# Run with docker-compose
docker-compose up -d
```

## What's Included

The Docker container includes:

- **C++ ASTERIX decoder** - High-performance command-line tool
- **Python module** - `asterix_decoder` package for Python 3.10+
- **Radar integration examples** - 7 ASTERIX encoders (CAT001, 019, 020, 021, 034, 048, 062)
- **Mock radar generator** - Synthetic radar plot generation
- **Visualization tools** - Statistics, plotting, benchmarks (optional)
- **Flight simulator integration** - ArduPilot SITL, JSBSim support (optional)

## Image Details

- **Base image:** Ubuntu 22.04 LTS
- **Size:** ~300-400 MB (multi-stage build)
- **Python version:** 3.10
- **C++ standard:** C++17 (Linux compatibility)
- **Security:** Non-root user (`asterix`), minimal attack surface
- **Architecture:** linux/amd64 (multi-arch support planned)

## Services

### 1. ASTERIX Decoder Service

Main service for running the ASTERIX decoder/encoder CLI.

```bash
# Start service
docker-compose up -d asterix

# Execute commands in container
docker-compose exec asterix asterix --version
docker-compose exec asterix python3 -c "import asterix; print(asterix.__version__)"

# Run radar integration examples
docker-compose exec asterix python3 /home/asterix/radar_integration/basic_mock_radar.py

# Stop service
docker-compose down
```

### 2. Jupyter Notebook Service (Optional)

Interactive notebook environment for exploring ASTERIX data.

```bash
# Start notebook service
docker-compose up -d notebook

# Access notebook at http://localhost:8888
# Token will be shown in logs: docker-compose logs notebook

# View logs to get token
docker-compose logs notebook | grep "token="
```

## Usage Examples

### Decode ASTERIX File

```bash
# From host machine (mount data directory)
docker run --rm -v $(pwd)/data:/home/asterix/data asterix \
  asterix -f /home/asterix/data/sample.pcap -j

# Using docker-compose
docker-compose exec asterix asterix -f /home/asterix/data/sample.pcap -jh
```

### Run Radar Integration Examples

```bash
# Basic mock radar
docker-compose exec asterix python3 /home/asterix/radar_integration/basic_mock_radar.py

# Aircraft scenario (5 aircraft, 60s simulation)
docker-compose exec asterix python3 /home/asterix/radar_integration/aircraft_scenario.py

# Complete surveillance demo (multi-category)
docker-compose exec asterix python3 /home/asterix/radar_integration/complete_surveillance_demo.py

# Encode and decode with validation
docker-compose exec asterix python3 /home/asterix/radar_integration/encode_and_decode.py
```

### Python API Usage

```bash
# Interactive Python session
docker-compose exec asterix python3

# In Python shell:
>>> import asterix
>>> data = open('/home/asterix/data/sample.raw', 'rb').read()
>>> records = asterix.parse(data)
>>> print(len(records))
```

### UDP Multicast Reception

```bash
# Run in host network mode for multicast support
docker run --rm --network host asterix \
  asterix -i 239.255.1.1:eth0:5000

# Or use docker-compose (already configured for host network)
docker-compose exec asterix asterix -i 239.255.1.1:eth0:5000
```

### Custom Category Definitions

```bash
# Mount custom config directory
docker run --rm \
  -v $(pwd)/config:/home/asterix/config:ro \
  asterix asterix -f /home/asterix/data/sample.ast

# Or use docker-compose (already configured)
# Place XML files in ./config/ directory
```

## Build Options

### Standard Build

```bash
docker build -t asterix:latest .
```

### Build with Build Arguments

```bash
# Specify Python version (if using different base image)
docker build --build-arg PYTHON_VERSION=3.12 -t asterix:3.12 .

# Enable build-time tests (slower but validates build)
docker build --build-arg RUN_TESTS=1 -t asterix:tested .
```

### Build for Different Architecture

```bash
# Build for ARM64 (e.g., Raspberry Pi 4, Apple Silicon)
docker buildx build --platform linux/arm64 -t asterix:arm64 .

# Multi-arch build
docker buildx build --platform linux/amd64,linux/arm64 -t asterix:multi .
```

## Volume Mounts

The docker-compose configuration uses the following volume mounts:

| Host Path | Container Path | Purpose |
|-----------|---------------|---------|
| `./data` | `/home/asterix/data` | ASTERIX data files (read/write) |
| `./config` | `/home/asterix/config` | Custom category definitions (read-only) |
| `./examples` | `/home/asterix/examples` | Example scripts (read-only) |
| `./notebooks` | `/home/asterix/notebooks` | Jupyter notebooks (read/write) |

Create these directories on the host:

```bash
mkdir -p data config notebooks
```

## Installing Optional Dependencies

The base image includes minimal dependencies. Install extras as needed:

```bash
# Matplotlib for visualization
docker-compose exec asterix pip3 install matplotlib

# PyMAVLink for ArduPilot integration
docker-compose exec asterix pip3 install pymavlink

# JSBSim for flight simulation
docker-compose exec asterix pip3 install jsbsim

# All optional dependencies
docker-compose exec asterix pip3 install matplotlib pymavlink jsbsim
```

Or build a custom image with extras:

```dockerfile
FROM asterix:latest

USER root
RUN pip3 install --no-cache-dir matplotlib pymavlink jsbsim
USER asterix
```

## Performance Tuning

### Resource Limits

Adjust resource limits in `docker-compose.yml`:

```yaml
services:
  asterix:
    deploy:
      resources:
        limits:
          cpus: '4.0'      # Increase CPU limit
          memory: 2G       # Increase memory limit
        reservations:
          cpus: '2.0'
          memory: 512M
```

### Shared Memory

For large-scale simulations, increase shared memory:

```bash
docker run --rm --shm-size=1g asterix ...
```

Or in docker-compose:

```yaml
services:
  asterix:
    shm_size: '1gb'
```

## Troubleshooting

### Import Error: No module named 'asterix'

**Cause:** Python module not installed or PYTHONPATH not set.

**Fix:**
```bash
# Verify installation
docker-compose exec asterix python3 -c "import asterix; print(asterix.__version__)"

# Check PYTHONPATH
docker-compose exec asterix env | grep PYTHONPATH

# Reinstall if needed
docker-compose exec asterix pip3 install asterix_decoder
```

### Permission Denied Errors

**Cause:** File ownership mismatch between host and container.

**Fix:**
```bash
# On host, fix ownership (asterix user is UID 1000 in container)
sudo chown -R 1000:1000 data/ config/ notebooks/

# Or run container as root (not recommended for security)
docker run --rm --user root asterix ...
```

### Multicast Not Working

**Cause:** Container not using host network.

**Fix:**
```bash
# Use host network mode
docker run --rm --network host asterix asterix -i 239.255.1.1:eth0:5000

# Or in docker-compose.yml, ensure:
services:
  asterix:
    network_mode: host
```

### Image Size Too Large

**Cause:** Build artifacts or cache bloat.

**Fix:**
```bash
# Rebuild without cache
docker build --no-cache -t asterix:latest .

# Check image size
docker images asterix

# Prune unused images and layers
docker system prune -a
```

## Testing

### Verify Build

```bash
# Test C++ decoder
docker run --rm asterix asterix --version

# Test Python module
docker run --rm asterix python3 -c "import asterix; print(asterix.__version__)"

# Test radar integration
docker run --rm asterix python3 /home/asterix/radar_integration/basic_mock_radar.py
```

### Run Unit Tests

```bash
# Python tests (if test suite included)
docker-compose exec asterix python3 -m unittest discover -s /home/asterix/lib

# Integration tests (requires test data)
docker-compose exec asterix bash -c "cd /usr/local/share/asterix/test && ./test.sh"
```

## Advanced Usage

### Shell Access

```bash
# Interactive shell
docker-compose exec asterix bash

# Run commands as root
docker-compose exec --user root asterix bash
```

### Debugging

```bash
# Enable verbose logging
docker-compose exec asterix asterix -v -f /home/asterix/data/sample.ast

# Check Python paths
docker-compose exec asterix python3 -m site

# List installed packages
docker-compose exec asterix pip3 list
```

### Custom Entry Point

```bash
# Run custom command
docker run --rm asterix python3 -c "print('Hello from ASTERIX')"

# Override entrypoint
docker run --rm --entrypoint /bin/bash asterix -c "ls -la /home/asterix"
```

## CI/CD Integration

### GitHub Actions Example

```yaml
name: Docker Build

on: [push]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build Docker image
        run: docker build -t asterix:${{ github.sha }} .
      - name: Test image
        run: |
          docker run --rm asterix:${{ github.sha }} asterix --version
          docker run --rm asterix:${{ github.sha }} python3 -c "import asterix"
```

### GitLab CI Example

```yaml
docker-build:
  stage: build
  image: docker:latest
  services:
    - docker:dind
  script:
    - docker build -t $CI_REGISTRY_IMAGE:$CI_COMMIT_SHA .
    - docker run --rm $CI_REGISTRY_IMAGE:$CI_COMMIT_SHA asterix --version
```

## Security Considerations

1. **Non-root user:** Container runs as user `asterix` (UID 1000)
2. **Minimal dependencies:** Only runtime dependencies installed
3. **Multi-stage build:** Build tools not included in final image
4. **Read-only mounts:** Config and examples mounted read-only
5. **No secrets:** No hardcoded credentials or API keys

## License

This Docker configuration is part of the ASTERIX decoder project (GPL-3.0-or-later).

See `LICENSE` for full license text.

## Support

- **Documentation:** https://github.com/montge/asterix
- **Issues:** https://github.com/montge/asterix/issues
- **Contributing:** See `CONTRIBUTING.md`

## Changelog

### v2.8.10 (2025-11-23)
- Initial Docker containerization
- Multi-stage build for smaller image size
- Jupyter notebook service support
- UDP multicast support via host network
- Non-root user for security
- Comprehensive radar integration examples included
