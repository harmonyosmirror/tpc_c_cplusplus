# Monocypher Third-party Library

## Overview

Monocypher is a small cryptographic library designed to be easy to deploy. This adaptation builds Monocypher as a standalone shared library for OpenHarmony so dependent libraries can link against it dynamically.

## Version

- 3.1.3

## Adapted Features

- Core Monocypher cryptographic primitives
- Optional Ed25519 support required by libcose
- Shared library output: libmonocypher.so

## Constraints

- [IDE and SDK version requirements](../../docs/constraint.md)

## Build

Run the lycium build script from the repository root:

```bash
cd lycium
./build.sh monocypher
```

After a successful build, the artifacts are installed under:

```text
lycium/usr/monocypher/<ARCH>/lib/libmonocypher.so
lycium/usr/monocypher/<ARCH>/include/monocypher.h
lycium/usr/monocypher/<ARCH>/include/optional/monocypher-ed25519.h
```

## Integration

- [Application HAP package integration](docs/hap_integrate.md)
