# Amiga Remakes

1:1 remakes of classic Amiga demos from the 1980s, reverse-engineered from original M68K assembly.

## Building

Requires both GCC (for Linux binaries) and MinGW (for Windows binaries).

**Windows users:** WSL2 (Windows Subsystem for Linux) is the recommended development environment, as it provides a full Linux toolchain including both GCC and MinGW cross-compilation support. Tested on WSL2 with Arch Linux (available directly from the Microsoft Store).

First-time setup:
```bash
./bootstrap.sh
```

Then build all demos:
```bash
./build_all.sh release
```

Built executables are placed in the `/bin` directory at the project root.

Individual demos can also be built from their respective directories, producing `remake` and `remake.exe` locally.

## Status

Active development. Multiple demos are worked on in parallel - some are complete, others may be incomplete or non-functional. No guarantees anything works at any given commit.

## License

MIT (see LICENSE file)
