#!/usr/bin/env python3
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
将目录内随包 .so 的 DT_NEEDED / DT_SONAME 改为无「.so 后数字后缀」形式，
并把文件合并为 libfoo.so（适配部分鸿蒙环境动态链接器对版本化文件名的限制）。

仅缩短字符串（新名不长于旧名），在 .dynstr 中原地覆盖，不增长 dynamic 段。
系统库名（libc、libstdc++.so.6 等）不修改。
"""
from __future__ import annotations

import mmap
import os
import re
import struct
import sys
from typing import BinaryIO, Dict, List, Optional, Tuple

# 仅去掉「.so」之后的数字版本尾缀；不动 libfoo.so.bar 等非数字尾缀
_SO_VER_SUFFIX = re.compile(r"^(?P<base>lib.+\.so)(\.\d[\d.]*)?$")

EI_CLASS = 4
ELFCLASS64 = 2
ELFCLASS32 = 1
ET_DYN = 3
PT_LOAD = 1
PT_DYNAMIC = 2
DT_NULL = 0
DT_NEEDED = 1
DT_STRTAB = 5
DT_STRSZ = 10
DT_SONAME = 14

# 与 copy_closure_so_to_libsmb_ohos.sh 中 is_system_soname 对齐
SYSTEM_SONAMES = frozenset(
    {
        "libc.so",
        "libm.so",
        "libdl.so",
        "libpthread.so",
        "libresolv.so",
        "librt.so",
        "libutil.so",
        "ld-linux-aarch64.so.1",
        "ld-linux-armhf.so.3",
        "libstdc++.so.6",
        "libgcc_s.so.1",
    }
)


def is_system_soname(name: str) -> bool:
    if name in SYSTEM_SONAMES:
        return True
    for p in ("libc.so", "libm.so", "libdl.so", "libpthread.so"):
        if name == p or name.startswith(p + "."):
            return True
    return False


def short_soname(name: str) -> str:
    """libfoo.so / libfoo.so.1 / libfoo.so.1.2.3 -> libfoo.so（系统库原样返回）。"""
    if is_system_soname(name):
        return name
    m = _SO_VER_SUFFIX.match(name)
    if not m:
        return name
    return m.group("base")


def is_elf_so(path: str) -> bool:
    try:
        with open(path, "rb") as f:
            hdr = f.read(5)
    except OSError:
        return False
    if len(hdr) < 5 or hdr[:4] != b"\x7fELF":
        return False
    return hdr[4] in (ELFCLASS32, ELFCLASS64)


def parse_ehdr(f: BinaryIO) -> Tuple[int, int, int, int, int, int]:
    f.seek(0)
    e_ident = f.read(16)
    if len(e_ident) < 16 or e_ident[:4] != b"\x7fELF":
        raise ValueError("not ELF")
    ei_class = e_ident[EI_CLASS]
    if ei_class == ELFCLASS64:
        fmt = "<HHIQQQIHHHHHH"
        rest = f.read(struct.calcsize(fmt))
        if len(rest) < struct.calcsize(fmt):
            raise ValueError("truncated ELF64 Ehdr")
        (
            e_type,
            e_machine,
            e_version,
            e_entry,
            e_phoff,
            e_shoff,
            e_flags,
            e_ehsize,
            e_phentsize,
            e_phnum,
            e_shentsize,
            e_shnum,
            e_shstrndx,
        ) = struct.unpack(fmt, rest)
        return ei_class, e_type, e_phoff, e_phentsize, e_phnum, e_ehsize
    if ei_class == ELFCLASS32:
        fmt = "<HHIIIIIHHHHHH"
        rest = f.read(struct.calcsize(fmt))
        if len(rest) < struct.calcsize(fmt):
            raise ValueError("truncated ELF32 Ehdr")
        (
            e_type,
            e_machine,
            e_version,
            e_entry,
            e_phoff,
            e_shoff,
            e_flags,
            e_ehsize,
            e_phentsize,
            e_phnum,
            e_shentsize,
            e_shnum,
            e_shstrndx,
        ) = struct.unpack(fmt, rest)
        return ei_class, e_type, e_phoff, e_phentsize, e_phnum, e_ehsize
    raise ValueError("unsupported ELF class")


def ph64(f: BinaryIO, e_phoff: int, e_phentsize: int, e_phnum: int) -> List[Tuple[int, int, int, int, int, int, int, int]]:
    out = []
    for i in range(e_phnum):
        f.seek(e_phoff + i * e_phentsize)
        chunk = f.read(56)
        if len(chunk) < 56:
            break
        p_type, p_flags, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align = struct.unpack(
            "<IIQQQQQQ", chunk
        )
        out.append((p_type, p_flags, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align))
    return out


def ph32(f: BinaryIO, e_phoff: int, e_phentsize: int, e_phnum: int) -> List[Tuple[int, int, int, int, int, int, int, int]]:
    out = []
    for i in range(e_phnum):
        f.seek(e_phoff + i * e_phentsize)
        chunk = f.read(32)
        if len(chunk) < 32:
            break
        p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_flags, p_align = struct.unpack(
            "<IIIIIIII", chunk
        )
        out.append((p_type, p_flags, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align))
    return out


def va_to_file_off(
    loads: List[Tuple[int, int, int, int, int, int, int, int]], va: int, is64: bool
) -> Optional[int]:
    for _pt, _fl, p_offset, p_vaddr, _pp, p_filesz, _pm, _al in loads:
        if p_vaddr <= va < p_vaddr + p_filesz:
            return p_offset + (va - p_vaddr)
    return None


def read_dyn64(
    f: BinaryIO, dyn_file_off: int
) -> Tuple[List[Tuple[int, int]], int, int]:
    entries: List[Tuple[int, int]] = []
    strtab_va = 0
    strsz = 0
    pos = dyn_file_off
    while True:
        f.seek(pos)
        chunk = f.read(16)
        if len(chunk) < 16:
            break
        d_tag, d_val = struct.unpack("<QQ", chunk)
        entries.append((d_tag, d_val))
        if d_tag == DT_NULL:
            break
        if d_tag == DT_STRTAB:
            strtab_va = d_val
        elif d_tag == DT_STRSZ:
            strsz = d_val
        pos += 16
    return entries, strtab_va, strsz


def read_dyn32(
    f: BinaryIO, dyn_file_off: int
) -> Tuple[List[Tuple[int, int]], int, int]:
    entries: List[Tuple[int, int]] = []
    strtab_va = 0
    strsz = 0
    pos = dyn_file_off
    while True:
        f.seek(pos)
        chunk = f.read(8)
        if len(chunk) < 8:
            break
        d_tag, d_val = struct.unpack("<II", chunk)
        entries.append((d_tag, d_val))
        if d_tag == DT_NULL:
            break
        if d_tag == DT_STRTAB:
            strtab_va = d_val
        elif d_tag == DT_STRSZ:
            strsz = d_val
        pos += 8
    return entries, strtab_va, strsz


def read_cstr(mm: memoryview, strtab_off: int, strsz: int, rel_off: int) -> str:
    start = strtab_off + rel_off
    if start >= strtab_off + strsz:
        return ""
    end = start
    while end < strtab_off + strsz and mm[end] != 0:
        end += 1
    return bytes(mm[start:end]).decode("ascii", errors="replace")


def patch_elf(path: str, dry_run: bool) -> int:
    with open(path, "r+b") as f:
        ei_class, e_type, e_phoff, e_phentsize, e_phnum, _eh = parse_ehdr(f)
        if e_type != ET_DYN:
            return 0
        loads = ph64(f, e_phoff, e_phentsize, e_phnum) if ei_class == ELFCLASS64 else ph32(
            f, e_phoff, e_phentsize, e_phnum
        )
        pt_loads = [x for x in loads if x[0] == PT_LOAD]
        dyn_ph = next((x for x in loads if x[0] == PT_DYNAMIC), None)
        if not dyn_ph:
            return 0
        _pt, _fl, dyn_off, dyn_va, _pp, _fs, _ms, _al = dyn_ph
        if ei_class == ELFCLASS64:
            entries, strtab_va, strsz = read_dyn64(f, dyn_off)
            is64 = True
        else:
            entries, strtab_va, strsz = read_dyn32(f, dyn_off)
            is64 = False
        if not strtab_va or not strsz:
            return 0
        strtab_file = va_to_file_off(pt_loads, strtab_va, is64)
        if strtab_file is None:
            return 0
        strtab_abs = int(strtab_file)

        mm = mmap.mmap(f.fileno(), 0)
        try:
            mv = memoryview(mm)
            changes = 0
            # Collect all DT_NEEDED/DT_SONAME string offsets for overlap check
            need_offsets = []
            for d_tag, d_val in entries:
                if d_tag not in (DT_NEEDED, DT_SONAME):
                    continue
                old = read_cstr(mv, strtab_abs, strsz, d_val)
                if not old:
                    continue
                need_offsets.append((d_val, old))
            for d_val, old in need_offsets:
                new = short_soname(old)
                if new == old:
                    continue
                o = old.encode("ascii")
                n = new.encode("ascii") + b"\x00"
                if len(n) > len(o) + 1:
                    sys.stderr.write(
                        f"{path}: skip (short name longer than old): {old!r} -> {new!r}\n"
                    )
                    continue
                start = strtab_abs + d_val
                if start + len(o) + 1 > strtab_abs + strsz:
                    continue
                # Check for overlapping string references
                for other_d_val, other_old in need_offsets:
                    if other_d_val == d_val:
                        continue
                    # Check if other string starts within [d_val, d_val + len(o)]
                    if d_val < other_d_val < d_val + len(o) + 1:
                        sys.stderr.write(
                            f"{path}: skip (overlapping string refs): {old!r} overlaps with {other_old!r}\n"
                        )
                        break
                else:
                    if not dry_run:
                        mv[start : start + len(n)] = n
                        if len(n) < len(o) + 1:
                            mv[start + len(n) : start + len(o) + 1] = b"\x00" * (len(o) + 1 - len(n))
                    changes += 1
            return changes
        finally:
            del mv
            mm.close()


def consolidate_files(directory: str, dry_run: bool) -> None:
    """将 libfoo.so.* 合并为单个 libfoo.so 文件。"""
    names = [n for n in os.listdir(directory) if n.startswith("lib") and ".so" in n]
    paths = [os.path.join(directory, n) for n in names if os.path.isfile(os.path.join(directory, n))]
    elves = [p for p in paths if is_elf_so(p)]
    groups: Dict[str, List[str]] = {}
    for p in elves:
        base = os.path.basename(p)
        groups.setdefault(short_soname(base), []).append(p)

    for short, members in groups.items():
        if is_system_soname(short):
            continue
        uniq = list({os.path.realpath(m): m for m in members}.values())
        if len(uniq) <= 1 and os.path.basename(uniq[0]) == short:
            continue
        # 选体积最大的 ELF 作为内容源（通常为完整版本 .so.x.y.z）
        def sz(x: str) -> int:
            try:
                return os.path.getsize(x)
            except OSError:
                return 0

        winner = max(uniq, key=sz)
        dst = os.path.join(directory, short)
        if dry_run:
            print(f"consolidate -> {short} from {winner}")
            continue
        with open(winner, "rb") as rf:
            data = rf.read()
        for m in uniq:
            try:
                os.remove(m)
            except OSError:
                pass
        with open(dst, "wb") as wf:
            wf.write(data)


def main() -> int:
    dry = "--dry-run" in sys.argv
    dirs = [a for a in sys.argv[1:] if not a.startswith("-")]
    if not dirs:
        print("usage: ohos_strip_so_versions.py [--dry-run] <dir>...", file=sys.stderr)
        return 2
    total = 0
    for d in dirs:
        if not os.path.isdir(d):
            print(f"skip (not dir): {d}", file=sys.stderr)
            continue
        consolidate_files(d, dry)
        for name in sorted(os.listdir(d)):
            path = os.path.join(d, name)
            if not os.path.isfile(path) or not is_elf_so(path):
                continue
            try:
                n = patch_elf(path, dry)
            except (ValueError, struct.error, OSError) as e:
                print(f"{path}: {e}", file=sys.stderr)
                continue
            total += n
    if dry:
        print(f"dry-run: would patch {total} dynamic string entries")
    else:
        print(f"patched {total} DT_NEEDED/DT_SONAME entries (in-place)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
