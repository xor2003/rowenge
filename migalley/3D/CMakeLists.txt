cmake_minimum_required(VERSION 3.16)
project(My3D LANGUAGES CXX)

# Build as a static library (consistent with solution usage)
add_library(My3D STATIC
    3DCODE.CPP
    3DCOM.CPP
    ANMRADIO.CPP
    BTREE.CPP
    COLLIDED.CPP
    IMAGEMAP.CPP
    LANDSCAP.CPP
    MIGLAND.CPP
    SHAPES.CPP
    SHPBLD.CPP
    TRANSITE.CPP
    VIEWSEL.CPP
    WEAPPAK.CPP
    WORLD.CPP
)

target_link_libraries(My3D PRIVATE ProjectDefaults)

# RERUN perf: the 3D pipeline (object traversal, landscape, scan-conversion
# driver) dominates MiG frame time at -O0. The whole lib gets -O2; together
# with the tree-wide -fwrapv / -fno-delete-null-pointer-checks,
# -fno-strict-aliasing neutralises the classic 1999-era UB hazards that make
# blanket -O2 unsafe.
target_compile_options(My3D PRIVATE -O2 -fno-strict-aliasing)

