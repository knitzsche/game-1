project(game1)

set(EXE mygame)

find_package(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIR})
include(FindPkgConfig)
pkg_check_modules(SDL2_GFX REQUIRED SDL2_gfx)
include_directories(${SDL2_GFX_INCLUDE_DIRS})
message(" sdl2 gx libs: " ${SDL2_GFX_LIBRARIES})
message(" sdl2 gfx dir: " ${SDL2_GFX_INCLUDE_DIRS})
add_executable(${EXE} src/main.cpp)
target_link_libraries(${EXE} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_GFX_LIBRARIES})
install(TARGETS ${EXE} RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin)

