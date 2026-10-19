# ENet 1.3.18 (MIT) — vendored snapshot for the DirectPlay transport.
# Reliable ordered UDP replaces the hand-rolled TCP session channel.
cmake_minimum_required(VERSION 3.16)
project(rowan_enet LANGUAGES C)

add_library(rowan_enet STATIC
    callbacks.c
    compress.c
    host.c
    list.c
    packet.c
    peer.c
    protocol.c
)
if(WIN32)
    target_sources(rowan_enet PRIVATE win32.c)
    target_link_libraries(rowan_enet PUBLIC winmm ws2_32)
else()
    target_sources(rowan_enet PRIVATE unix.c)
endif()
target_include_directories(rowan_enet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# vendored code — keep its warnings out of the project's error surface
target_compile_options(rowan_enet PRIVATE -w)
