# rowan_smacker — vendored libsmacker (SMK video decode; replaces the
# ffmpeg/ffprobe pipes in WINSMACK.CPP).
cmake_minimum_required(VERSION 3.16)
project(rowan_smacker LANGUAGES C)

add_library(rowan_smacker STATIC smacker.c)
target_include_directories(rowan_smacker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
