cmake_minimum_required(VERSION 3.16)
project(Hardware LANGUAGES CXX)

add_library(Hardware STATIC
    # Hardware source files
    dinput_stub.cpp
    CONFIG.CPP
    DDRWINIT.CPP
    HARDPUB.CPP
    HARDWIN.CPP
    MESSENGN.CPP
    MILES.CPP
    RCHATTER.CPP
    SFONTS.CPP
    WIN3D.CPP
    direct_3d.cpp
    XX_ScreenFlip_Vulkan.cpp
)

target_link_libraries(Hardware PRIVATE ProjectDefaults)

# XX_ScreenFlip_Vulkan.cpp is modern port code (Vulkan/SDL upload path) —
# its per-frame RGB565->BGRA staging loop benefits from optimisation and it
# carries none of the 1999-era aliasing hazards the rest of the tree may.
set_source_files_properties(XX_ScreenFlip_Vulkan.cpp
    PROPERTIES COMPILE_OPTIONS "-O2")

