cmake_minimum_required(VERSION 3.23)
project(3dgraph
  VERSION 1.0
  DESCRIPTION "a very basic opengl render"
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
find_package(range-v3 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glm REQUIRED)

set_source_files_properties(src/glad.c PROPERTIES LANGUAGE CXX)
add_executable(${PROJECT_NAME} src/main.cpp src/shader.cpp src/shader_program.cpp src/glad.c src/gl_inspect.cpp)

target_link_libraries(${PROJECT_NAME}
  PRIVATE range-v3::range-v3
  PRIVATE SDL2::SDL2
  PRIVATE SDL2::SDL2main
  PRIVATE glm::glm)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME}
  PRIVATE ${3dgraph_SOURCE_DIR}/include
)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  ADD_CUSTOM_TARGET(link_compile_commands_json ALL
                    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/compile_commands.json ${3dgraph_SOURCE_DIR}/compile_commands.json)
endif()
