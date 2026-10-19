# Unit tests for the native MiG Alley port (32-bit).
#
# Exercises the real 1998 code paths (shape instruction handlers, matrix
# clip/projection math, palette selection, fixed-point union layout) through
# the same static libraries the game links.

add_executable(unit_tests
    unit_tests.cpp
    test_abi.cpp
    test_compat.cpp
    ${CMAKE_SOURCE_DIR}/MigAlley/MigCString.cpp
)

target_link_libraries(unit_tests PRIVATE ProjectDefaults)

target_link_libraries(unit_tests PRIVATE
 -Wl,--start-group
    My3D
    Assembler
    AI
    Bfields
    Overlay
    Comms
    Files
    General
    Graphics
    Input
    Math
    Movecode
    Missman
    Hardware
    RowanMFC
    RowanAFX
    RowanSheets
    CollisionsEtAl
    Model
    Aircraft
 -Wl,--end-group
)

add_test(NAME unit_tests COMMAND unit_tests)

# widget_tests — headless tests for the ported OCX widget layer
# (CRCombo dropdown placement, CListBox, CRSpinBut, CRListBox, edits,
# buttons, scrollbars, tabs…). Runs on SDL's dummy video driver, so no
# Xvfb needed. Asserts original-OCX semantics from bob/rcombo/*.cpp.
add_executable(widget_tests
    test_widgets.cpp
    ${CMAKE_SOURCE_DIR}/MigAlley/MigCString.cpp
)
target_link_libraries(widget_tests PRIVATE ProjectDefaults)
target_link_libraries(widget_tests PRIVATE
 -Wl,--start-group
    My3D
    Assembler
    AI
    Bfields
    Overlay
    Comms
    Files
    General
    Graphics
    Input
    Math
    Movecode
    Missman
    Hardware
    RowanMFC
    RowanAFX
    RowanSheets
    CollisionsEtAl
    Model
    Aircraft
 -Wl,--end-group
)
add_test(NAME widget_tests COMMAND widget_tests)

# The 32-bit sysroot SDL/mixer libs aren't on the default loader path.
set_tests_properties(unit_tests PROPERTIES
    ENVIRONMENT "LD_LIBRARY_PATH=${ROWAN_SYSROOT}/usr/lib/i386-linux-gnu"
)
