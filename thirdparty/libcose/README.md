# libcose Third-party Library

## Overview

libcose is a C99 implementation of the COSE (CBOR Object Signing and Encryption, RFC 8152) standard, targeting resource-constrained devices (embedded / IoT). Its core design goal is zero dynamic memory allocation — all buffers are supplied by the caller.

## Version

- ea1fed8 (master branch, commit ea1fed87d6ca9b478f8bed323af97e6b192c0a6d)

## Adapted Features

- Ed25519 signing and verification (EdDSA)
- X25519 key exchange
- ChaCha20-Poly1305 AEAD authenticated encryption
- HKDF-SHA512 key derivation
- External Payload and Additional Authenticated Data (AAD) support
- Compile-time configurable limits: max signatures, recipients, headers, and message size
- Monocypher 3.1.3 as cryptographic backend (pure C, linked as an independent shared library)

## Constraints

- [IDE and SDK version requirements](../../docs/constraint.md)

## Build

Run the lycium build script from the repository root:

```bash
cd lycium
./build.sh libcose
```

The build depends on `nanocbor`, `monocypher`, and `CUnit`. The lycium build script resolves and builds these dependencies before building `libcose`.

## Usage

Before calling the key generation, signing, or encryption APIs of `libcose`, register a random number callback through `cose_crypt_set_rng()`. On HarmonyOS devices, `getrandom()` or `/dev/urandom` is recommended as the random source. For details, see [Application HAP package integration](docs/hap_integrate.md#random-number-callback-registration).

## Integration

- [Application HAP package integration](docs/hap_integrate.md)
