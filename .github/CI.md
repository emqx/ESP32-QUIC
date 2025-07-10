# CI/CD Pipeline Documentation

This document describes the Continuous Integration (CI) pipeline setup for the ESP32-QUIC project.

## Overview

The CI pipeline is designed to build and test the ESP32-QUIC demo project across different ESP-IDF versions, ensuring code quality and build reliability.

## Workflows

### 1. Main CI Workflow (`.github/workflows/ci.yml`)

**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` branch
- Manual workflow dispatch

**Features:**
- Builds project with ESP-IDF v5.2 for ESP32-C3 target
- Automatically applies required patches:
  - wolfSSL NO_SESSION_CACHE fix (resolves build issue mentioned in README)
  - ngtcp2 patch if present
- Archives build artifacts for download
- Handles submodule dependencies

### 2. Matrix Build Workflow (`.github/workflows/build-matrix.yml`)

**Purpose:** Validates compatibility across multiple ESP-IDF versions

**Matrix Testing:**
- ESP-IDF v5.1
- ESP-IDF v5.2
- ESP-IDF v6.0 (experimental)

**Benefits:**
- Ensures forward/backward compatibility
- Separate artifacts per ESP-IDF version
- Parallel execution for faster feedback

### 3. Test Workflow (`.github/workflows/test.yml`)

**Components:**
- **Unit Tests:** Runs pytest-based tests for basic functionality
- **Code Quality:** Basic linting and formatting checks

## Automatic Fixes Applied

### wolfSSL Session Cache Issue

The CI automatically resolves the common build error:
```
undefined reference to 'TlsSessionCacheGetAndRdLock'
```

**Solution:** Comments out `#define NO_SESSION_CACHE` in wolfSSL configuration:
```bash
sed -i 's/#define NO_SESSION_CACHE/\/\/#define NO_SESSION_CACHE/' managed_components/wolfssl__wolfssl/include/user_settings.h
```

### ngtcp2 Patch Application

If `ngtcp2.patch` exists, it's automatically applied to resolve compatibility issues.

## Configuration Files

### `sdkconfig.ci`

Contains CI-optimized configuration:
- ESP32-C3 target specification
- Optimized flash settings for faster builds
- Required network components for QUIC functionality

### `pytest_quic_demo.py`

ESP32-C3 specific tests:
- Startup validation
- Memory usage checks
- QEMU compatibility tests

## Build Artifacts

Each successful build produces:
- `*.bin` - Firmware binary files
- `*.elf` - Executable with debug symbols  
- `*.map` - Memory layout mapping
- `partition_table/` - Partition table files

Artifacts are available for download for 7 days after build completion.

## Usage

### Running CI Locally

To replicate CI behavior locally:

1. **Install ESP-IDF v5.2**
2. **Clone with submodules:**
   ```bash
   git clone --recursive https://github.com/emqx/ESP32-QUIC.git
   ```
3. **Apply patches (automatic in CI):**
   ```bash
   # Apply ngtcp2 patch if needed
   cd components/ngtcp2/ngtcp2
   git apply ../../../ngtcp2.patch
   cd ../../..
   
   # Apply wolfSSL patch after reconfigure
   idf.py reconfigure
   sed -i 's/#define NO_SESSION_CACHE/\/\/#define NO_SESSION_CACHE/' managed_components/wolfssl__wolfssl/include/user_settings.h
   ```
4. **Build:**
   ```bash
   idf.py build
   ```

### Manual Workflow Dispatch

All workflows can be triggered manually from the GitHub Actions tab for testing purposes.

## Troubleshooting

### Common Issues

1. **Submodule Errors:** Ensure `submodules: 'recursive'` is properly configured
2. **wolfSSL Build Errors:** The CI automatically applies the NO_SESSION_CACHE fix
3. **Memory Issues:** The project uses custom partition table for larger app size
4. **Component Dependencies:** CI runs `idf.py reconfigure` to install managed components

### CI Failure Debugging

1. Check the build logs for specific error messages
2. Verify that patches were applied correctly
3. Ensure ESP-IDF version compatibility
4. Check for any new component dependency requirements

## References

- [ESP-IDF CI Action](https://github.com/espressif/esp-idf-ci-action)
- [ESP32-QUIC Project README](../README.md)
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)