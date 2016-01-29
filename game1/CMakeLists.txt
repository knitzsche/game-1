project(game1)

find_package(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIR})

add_executable(game1 src/main.cpp)
target_link_libraries(game1 ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY})
install(TARGETS game1 RUNTIME DESTINATION ${BIN_DIR})

