cmake_minimum_required(VERSION 3.16)
project(Graphics LANGUAGES CXX)

add_library(Graphics STATIC
    # Graphics source files
    GRAFPRIM.CPP
    LBM.CPP
    POLYGON.CPP
    TGA.CPP
    WRAPPER.CPP


)

target_link_libraries(Graphics PRIVATE ProjectDefaults)

# GRAFPRIM.CPP hosts the per-pixel span fillers (bil_color/bil_despeckle
# inner loops) — the dominant 3D cost. It is self-contained math with no
# legacy aliasing tricks, so it is safe to optimise even though the rest of
# the 1999-era tree deliberately builds unoptimised.
set_source_files_properties(GRAFPRIM.CPP
    PROPERTIES COMPILE_OPTIONS "-O2")

# POLYGON.CPP hosts drawpoly/softpoly and the per-scanline converters —
# sampled hot next to the span fillers. Same -O2 carve-out (with
# -fno-strict-aliasing as it is 1998-era traversal code).
set_source_files_properties(POLYGON.CPP
    PROPERTIES COMPILE_OPTIONS "-O2;-fno-strict-aliasing")

