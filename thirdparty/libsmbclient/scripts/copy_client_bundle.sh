#!/usr/bin/env bash
# Pack libsmbclient readelf transitive closure + Lycium depends into
#   <install_prefix>/client/lib/   (flat .so bundle for HAP integration)
#   <install_prefix>/client/include/libsmbclient.h
#
# Full waf install under <install_prefix>/ is left untouched for inspection.
#
# Usage (called from HPKBUILD package()):
#   copy_client_bundle.sh <install_prefix> <arch> <lycium_root>
#
# Env:
#   OHOS_STRIP_SO_VERSIONS=0  skip ohos_strip_so_versions.py (default: 1)
set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ $# -lt 3 ]]; then
  echo "usage: $0 <install_prefix> <arch> <lycium_root>" >&2
  exit 2
fi

OHOS_PREFIX="$(cd "$1" && pwd)"
ARCH="$2"
LYCIUM_ROOT="$(cd "$3" && pwd)"
CLIENT_ROOT="${OHOS_PREFIX}/client"
CLIENT_LIB="${CLIENT_ROOT}/lib"
CLIENT_INC="${CLIENT_ROOT}/include"
OHOS_STRIP_SO_VERSIONS="${OHOS_STRIP_SO_VERSIONS:-1}"

# Same order as HPKBUILD depends (for LIBDIRS / pkg-config roots).
LYCIUM_DEPS=(zlib gmp nettle_3_9_1 libtasn1_4_19_0 gnutls)

LIBDIRS=("${OHOS_PREFIX}/lib" "${OHOS_PREFIX}/lib/private")
for dep in "${LYCIUM_DEPS[@]}"; do
  d="${LYCIUM_ROOT}/usr/${dep}/${ARCH}/lib"
  if [[ -d "$d" ]]; then
    LIBDIRS+=("$d")
  fi
done

declare -A SYSNAME
for s in libc.so libm.so libdl.so libpthread.so libresolv.so librt.so libutil.so \
         libstdc++.so.6 libgcc_s.so.1; do
  SYSNAME["$s"]=1
done
if [[ "$ARCH" == "armeabi-v7a" ]]; then
  SYSNAME["ld-linux-armhf.so.3"]=1
else
  SYSNAME["ld-linux-aarch64.so.1"]=1
fi

is_system_soname() {
  local n="$1"
  [[ -n "${SYSNAME[$n]:-}" ]] && return 0
  [[ "$n" == libc.so* || "$n" == libm.so* || "$n" == libdl.so* || "$n" == libpthread.so* ]] && return 0
  return 1
}

resolve_soname() {
  local soname="$1" d p
  for d in "${LIBDIRS[@]}"; do
    p="${d}/${soname}"
    if [[ -e "$p" ]]; then
      readlink -f "$p"
      return 0
    fi
  done
  return 1
}

find_start() {
  local c p
  for c in libsmbclient.so.0 libsmbclient.so libsmbclient.so.0.8.1; do
    if p="${OHOS_PREFIX}/lib/${c}"; [[ -e "$p" ]]; then
      readlink -f "$p"
      return 0
    fi
  done
  return 1
}

START="$(find_start)" || {
  echo "ERROR: libsmbclient: libsmbclient.so not found under ${OHOS_PREFIX}/lib" >&2
  exit 1
}

declare -A SEEN
declare -A EXTERNAL
QUEUE=("$START")
SEEN["$START"]=1

while ((${#QUEUE[@]} > 0)); do
  CUR="${QUEUE[0]}"
  QUEUE=("${QUEUE[@]:1}")
  [[ -f "$CUR" ]] || continue
  while IFS= read -r need || [[ -n "$need" ]]; do
    [[ -z "$need" ]] && continue
    if is_system_soname "$need"; then
      continue
    fi
    if RES=$(resolve_soname "$need"); then
      if [[ -z "${SEEN[$RES]:-}" ]]; then
        SEEN["$RES"]=1
        QUEUE+=("$RES")
      fi
    else
      EXTERNAL["$need"]=1
    fi
  done < <(readelf -d "$CUR" 2>/dev/null | sed -n 's/.*Shared library: \[\([^]]*\)\].*/\1/p')
done

rm -rf "${CLIENT_LIB}"
mkdir -p "${CLIENT_LIB}" "${CLIENT_INC}"

for k in "${!SEEN[@]}"; do
  cp -a "$k" "${CLIENT_LIB}/$(basename "$k")"
done

# Resolve remaining EXTERNAL names from Lycium depends (e.g. gnutls/hogweed not yet in SEEN).
if ((${#EXTERNAL[@]} > 0)); then
  for need in "${!EXTERNAL[@]}"; do
    if RES=$(resolve_soname "$need"); then
      cp -a "$RES" "${CLIENT_LIB}/$(basename "$RES")"
      unset "EXTERNAL[$need]"
    fi
  done
fi

if ((${#EXTERNAL[@]} > 0)); then
  echo "ERROR: libsmbclient client bundle: unresolved NEEDED (non-system):" >&2
  for e in $(printf '%s\n' "${!EXTERNAL[@]}" | sort -u); do
    echo "  $e" >&2
  done
  exit 1
fi

# Materialize symlinks to regular files (portable / Windows-friendly trees).
while IFS= read -r lnk; do
  tgt="$(readlink "$lnk" || true)"
  src=""
  if [[ -n "$tgt" && -e "$(dirname "$lnk")/$tgt" ]]; then
    src="$(dirname "$lnk")/$tgt"
  elif [[ -n "$tgt" ]]; then
    for d in "${LIBDIRS[@]}"; do
      if [[ -e "$d/$(basename "$tgt")" ]]; then
        src="$d/$(basename "$tgt")"
        break
      fi
    done
  fi
  if [[ -n "$src" ]]; then
    cp -L "$src" "${lnk}.tmp_real"
    rm -f "$lnk"
    mv -f "${lnk}.tmp_real" "$lnk"
  fi
done < <(find "$CLIENT_LIB" -maxdepth 1 -type l)

# SONAME alias files when missing (e.g. libfoo.so.0).
for so in "$CLIENT_LIB"/*.so*; do
  [[ -f "$so" ]] || continue
  soname=$(readelf -d "$so" 2>/dev/null | sed -n 's/.*SONAME.*\[\([^]]*\)\].*/\1/p' | head -n 1)
  if [[ -n "${soname:-}" && ! -e "$CLIENT_LIB/$soname" ]]; then
    cp -f "$so" "$CLIENT_LIB/$soname"
  fi
done

if [[ "$OHOS_STRIP_SO_VERSIONS" == "1" ]]; then
  python3 "${SCRIPT_DIR}/ohos_strip_so_versions.py" "$CLIENT_LIB"
else
  echo "==> libsmbclient: skipped ohos_strip_so_versions (OHOS_STRIP_SO_VERSIONS=0)"
fi

if [[ -f "${OHOS_PREFIX}/include/libsmbclient.h" ]]; then
  cp -a "${OHOS_PREFIX}/include/libsmbclient.h" "${CLIENT_INC}/"
else
  echo "WARN: libsmbclient: ${OHOS_PREFIX}/include/libsmbclient.h not found" >&2
fi

_n="$(find "$CLIENT_LIB" -mindepth 1 -maxdepth 1 | wc -l)"
echo "==> libsmbclient client bundle: ${CLIENT_LIB} (${_n} items), header -> ${CLIENT_INC}/"
