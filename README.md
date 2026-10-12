# rowan-engine

Unified source tree for the Rowan Software flight-sim ports:
**Battle of Britain (2000)** and **MiG Alley (1999)**.

This repo consolidates what used to be three separate trees, each with its
full git history preserved (see the "Relocate ..." merge commits):

```
migalley/   MiG Alley port + shared MFC/SDL compat layer (main build tree;
            also produces the `bob` executable target)
engine/     rowan-engine — DX7/DDraw software + Vulkan compat layer,
            shared shell, unit tests, migration/compare tooling
bob/        bob-flight-sim — BoB game sources (VS2013 port lineage)
```

## Build (64-bit — canonical)

```sh
# dependencies: amd64 -dev packages (see .github/workflows/ci.yml) or a local
# sysroot: point ROWAN_SYSROOT at a dir with usr/{include,lib}/x86_64-linux-gnu
# (default lookup: ./sysroot-amd64, a symlink works)
cmake -S migalley -B build64 -G Ninja        # ROWAN_64BIT=ON is the default
cmake --build build64 -j$(nproc)
# outputs: build64/MigAlley/MigAlley, build64/Bob/bob
```

### Deprecated: 32-bit build

The 32-bit build is kept only as a diagnostic baseline while LP64 migration
issues surface. It needs i386 -dev packages (or a `sysroot-i386` dir), and
CMake emits a deprecation notice:

```sh
cmake -S migalley -B build -G Ninja -DROWAN_64BIT=OFF
cmake --build build -j$(nproc)
# sanitizer build (32-bit only): cmake -S migalley -B build-san -G Ninja -DROWAN_64BIT=OFF -DMIG_SANITIZERS=ON
```

## Run

```sh
./run-migalley.sh [gamedir]     # ROWAN_MIG_GAMEDIR / ROWAN_SYSROOT env
./run-bob.sh [gamedir]          # ROWAN_BOB_GAMEDIR / ROWAN_SYSROOT env
```

Game data (retail installs, wineprefixes) is NOT in this repo — point the
env vars at your install trees. Tests that need game data self-skip (77).

## Tests and lint

```sh
ctest --test-dir build64 --output-on-failure  # smoke_xvfb, lifecycle, units
ctest --test-dir engine/build                 # engine unit tests
(cd migalley && bash scripts/lint.sh [files])  # clang-check + cppcheck
```

## Layout notes

- `sysroot-i386/`, `sysroot-amd64/`, `runtime/` are gitignored local wiring.
- `migalley/H` and `bob/h` headers self-assert `#pragma pack` — do not rely
  on include order for structure packing.
- See `HANDOFF` docs in the archive for the full porting history.
