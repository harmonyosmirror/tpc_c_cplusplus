# NanoCBOR Third-party Library

## Overview

NanoCBOR is an extremely small CBOR (Concise Binary Object Representation, RFC 7049) encoder/decoder library designed for resource-constrained devices (embedded, IoT). It is optimized for 32-bit architectures while remaining compatible with 8-bit and 16-bit platforms. The decoder requires only 600-800 bytes of Flash on a Cortex-M0+. NanoCBOR is also a mandatory dependency of libcose.

## Version

- 0623c45 (master branch, commit 0623c45686f6bc296c35416f3a41a71b148122d7)

## Adapted Features

- CBOR data decoding (integers, byte strings, text strings, arrays, maps, tags, simple values, floats)
- CBOR data encoding (integers, byte strings, text strings, arrays, maps, tags, simple values, floats)
- Progressive decoding of nested containers (arrays/maps)
- Zero dynamic memory allocation; all buffers provided by the caller
- Only dependency: endian conversion functions (can use `__builtin_bswap32`/`__builtin_bswap64`)

## Constraints

- [IDE and SDK version requirements](../../docs/constraint.md)

## Integration

- [Application HAP package integration](docs/hap_integrate.md)
