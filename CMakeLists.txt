cmake_minimum_required(VERSION 3.23)
project(3dgraph
  VERSION 1.0
  DESCRIPTION "a very basic opengl render"
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

find_package(cpptrace REQUIRED)
find_package(fmt REQUIRED)
find_package(glm REQUIRED)
find_package(opengl_system REQUIRED)
find_package(SDL3 REQUIRED)
find_package(spdlog REQUIRED)

set_source_files_properties(src/glad.c PROPERTIES LANGUAGE CXX)
add_executable(${PROJECT_NAME} src/main.cpp src/shader.cpp src/shader_program.cpp src/glad.c src/gl_inspect.cpp src/event_loop.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_SWIZZLE)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_SWIZZLE_XYZW)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_MESSAGES=GLM_ENABLE)

target_link_libraries(${PROJECT_NAME}
  PRIVATE cpptrace::cpptrace
  PRIVATE fmt::fmt
  PRIVATE glm::glm
  PRIVATE opengl::opengl
  PRIVATE SDL3::SDL3
  PRIVATE spdlog::spdlog)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME}
  PRIVATE ${3dgraph_SOURCE_DIR}/include
)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  ADD_CUSTOM_TARGET(link_compile_commands_json ALL
                    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/compile_commands.json ${3dgraph_SOURCE_DIR}/compile_commands.json)
endif()
