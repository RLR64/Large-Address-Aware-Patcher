# Large Address Aware Redux

A modern C++23 CLI tool for setting, clearing, and checking the `IMAGE_FILE_LARGE_ADDRESS_AWARE` flag on 32-bit Windows PE executables.

This allows a 32-bit process to access up to **4GB of virtual address space** on 64-bit Windows, instead of the default 2GB limit.

Built as a replacement for the original Large Address Aware tool, which may not work on versions later than Windows 11 25H2 as it deprecates some things that the previous tool relied on.

---

## Usage

```powershell
# Set the LAA flag (creates a .bak backup automatically)
& ".\Large Address Aware.exe" <path-to-exe>

# Check the current flag status without modifying
& ".\Large Address Aware.exe" <path-to-exe> --check

# Clear the LAA flag
& ".\Large Address Aware.exe" <path-to-exe> --clear

# Set the LAA flag without creating a backup
& ".\Large Address Aware.exe" <path-to-exe> --no-backup
```

### Example with Dungeon Siege (32-bit Game)

```powershell
& ".\Large Address Aware.exe" "C:\Program Files (x86)\Steam\steamapps\common\Dungeon Siege 1\DungeonSiege.exe"
```

```
----------------------------------------
 Large Address Aware Patcher
----------------------------------------

File          : C:\Program Files (x86)\Steam\steamapps\common\Dungeon Siege 1\DungeonSiege.exe
Architecture  : 32-bit
LAA Flag      : Disabled
Characteristics : 0x10f

[ OK ] LAA flag enabled successfully.
[ OK ] Backup created (.bak)
```

---

## Python Version

For those who prefer not to use the `.exe`, a Python version (`set_laa.py`) is included that provides identical behaviour and the same command-line interface.

```bash
python set_laa.py <path-to-exe>
python set_laa.py <path-to-exe> --check
python set_laa.py <path-to-exe> --clear
python set_laa.py <path-to-exe> --no-backup
```

Requires Python 3.10+. No external dependencies.

The C++ version was written to replicate the behaviour of the Python script exactly and the Python script was actually the original project idea before deciding to make it a CLI Windows tool.

### Example with Dungeon Siege (32-bit Game)

```powershell
python set_laa.py "C:\Program Files (x86)\Steam\steamapps\common\Dungeon Siege 1\DungeonSiege.exe"
```

```
File      : C:\Program Files (x86)\Steam\steamapps\common\Dungeon Siege 1\DungeonSiege.exe
Arch      : 32-bit (x86)
LAA flag  : SET
Chars     : 0x012F
[INFO] LAA flag is already set. Nothing to do.
```

---

## Building from Source

**Requirements:**
- CMake 4.3+
- Clang 22+ (or any C++23 capable compiler)
- Ninja
- Windows 11 SDK (10.0.26100+)

```powershell
# Debug
cmake --preset debug
cmake --build build/debug

# Release (fully static, no runtime dependencies)
cmake --preset release
cmake --build build/release
```

The release binary is fully static — only depends on `KERNEL32.dll` and runs on any Windows machine with no install required.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.