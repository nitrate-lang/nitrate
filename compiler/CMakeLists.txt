project(nitrate-compiler)

option(NITRATE_COMPILER_FUZZING "Enable compiler fuzzing targets" OFF)
option(NITRATE_COMPILER_SANITIZE "Enable compiler sanitizers" OFF)

add_subdirectory(lexer)
