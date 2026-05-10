#!/usr/bin/env python3
"""
Large Address Aware Patcher
Sets the IMAGE_FILE_LARGE_ADDRESS_AWARE flag (0x0020) in a 32-bit PE executable.
This allows the process to use up to 4GB of virtual address space on 64-bit Windows
instead of the default 2GB limit.

Usage:
    python set_laa.py <path_to_exe>          # Set LAA flag (makes a .bak backup)
    python set_laa.py <path_to_exe> --check  # Just check current flag status
    python set_laa.py <path_to_exe> --clear  # Clear the LAA flag
    python set_laa.py <path_to_exe> --no-backup  # Set without making a backup
"""

import sys
import os
import shutil
import struct
import argparse

# PE constants
DOS_SIGNATURE       = b'MZ'
PE_SIGNATURE        = b'PE\x00\x00'
IMAGE_FILE_LARGE_ADDRESS_AWARE = 0x0020

# Offsets
DOS_HEADER_LFANEW_OFFSET = 0x3C   # Offset to PE header offset (4 bytes at 0x3C)
COFF_CHARACTERISTICS_OFFSET = 18  # Bytes into COFF header (after PE sig + machine + numsections + timestamp + symtableptr + numsymbols)


def read_pe_characteristics(data: bytes) -> tuple[int, int]:
    """
    Returns (characteristics, characteristics_file_offset).
    Raises ValueError if not a valid PE.
    """
    # Check DOS header
    if data[:2] != DOS_SIGNATURE:
        raise ValueError("Not a valid executable: missing MZ signature.")

    # Get PE header offset
    pe_offset = struct.unpack_from('<I', data, DOS_HEADER_LFANEW_OFFSET)[0]

    # Check PE signature
    if data[pe_offset:pe_offset + 4] != PE_SIGNATURE:
        raise ValueError("Not a valid PE file: missing PE signature.")

    # Characteristics field is 18 bytes into the COFF header (right after the 4-byte PE sig)
    chars_offset = pe_offset + 4 + COFF_CHARACTERISTICS_OFFSET
    characteristics = struct.unpack_from('<H', data, chars_offset)[0]

    return characteristics, chars_offset


def check_32bit(data: bytes, pe_offset: int) -> bool:
    """Check if this is a 32-bit PE (IMAGE_FILE_MACHINE_I386 = 0x014c)."""
    machine = struct.unpack_from('<H', data, pe_offset + 4)[0]
    return machine == 0x014c


def patch_file(exe_path: str, action: str = 'set', make_backup: bool = True):
    """
    Patch the LAA flag in the given executable.
    action: 'set' | 'clear' | 'check'
    """
    if not os.path.isfile(exe_path):
        print(f"[ERROR] File not found: {exe_path}")
        sys.exit(1)

    with open(exe_path, 'rb') as f:
        data = bytearray(f.read())

    try:
        characteristics, chars_offset = read_pe_characteristics(data)
    except ValueError as e:
        print(f"[ERROR] {e}")
        sys.exit(1)

    # Grab PE offset for 32-bit check
    pe_offset = struct.unpack_from('<I', data, DOS_HEADER_LFANEW_OFFSET)[0]
    is_32bit = check_32bit(data, pe_offset)

    laa_set = bool(characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE)

    print(f"File      : {exe_path}")
    print(f"Arch      : {'32-bit (x86)' if is_32bit else '64-bit / other'}")
    print(f"LAA flag  : {'SET' if laa_set else 'NOT SET'}")
    print(f"Chars     : 0x{characteristics:04X}")

    if action == 'check':
        return

    if not is_32bit:
        print("\n[WARNING] This doesn't look like a 32-bit executable.")
        answer = input("Patch anyway? (y/N): ").strip().lower()
        if answer != 'y':
            print("Aborted.")
            sys.exit(0)

    if action == 'set':
        if laa_set:
            print("\n[INFO] LAA flag is already set. Nothing to do.")
            return
        new_chars = characteristics | IMAGE_FILE_LARGE_ADDRESS_AWARE
        verb = "Setting"
    elif action == 'clear':
        if not laa_set:
            print("\n[INFO] LAA flag is already clear. Nothing to do.")
            return
        new_chars = characteristics & ~IMAGE_FILE_LARGE_ADDRESS_AWARE
        verb = "Clearing"
    else:
        raise ValueError(f"Unknown action: {action}")

    if make_backup:
        backup_path = exe_path + '.bak'
        shutil.copy2(exe_path, backup_path)
        print(f"\n[INFO] Backup saved to: {backup_path}")

    # Write patched characteristics
    struct.pack_into('<H', data, chars_offset, new_chars)

    with open(exe_path, 'wb') as f:
        f.write(data)

    print(f"[OK]  {verb} LAA flag: 0x{characteristics:04X} -> 0x{new_chars:04X}")
    print(f"[OK]  Patched successfully: {exe_path}")


def main():
    parser = argparse.ArgumentParser(
        description="Set/clear/check the Large Address Aware flag on a PE executable."
    )
    parser.add_argument('exe', help='Path to the .exe file')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--check', action='store_true', help='Only check the current flag status')
    group.add_argument('--clear', action='store_true', help='Clear (remove) the LAA flag')
    parser.add_argument('--no-backup', action='store_true', help='Skip making a .bak backup')

    args = parser.parse_args()

    if args.check:
        action = 'check'
    elif args.clear:
        action = 'clear'
    else:
        action = 'set'

    patch_file(args.exe, action=action, make_backup=not args.no_backup)


if __name__ == '__main__':
    main()