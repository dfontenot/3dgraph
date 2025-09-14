cmake_minimum_required(VERSION 3.24)
project(3dgraph
  VERSION 1.0
  DESCRIPTION "a very basic opengl render"
  LANGUAGES CXX)

cmake_policy(SET CMP0132 NEW)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_BUILD_TYPE Debug)

# TODO: workaround for gcc version lagging the clang version by a lot on
# CI, TODO restrict this case a bit more
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "-stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++")
endif()

include(FetchContent)

# workaround for https://github.com/conan-io/conan-center-index/issues/25185
FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_SHALLOW 1
  GIT_TAG v1.15.3
  OVERRIDE_FIND_PACKAGE
)

# see https://github.com/gabime/spdlog/blob/5532231bbc31bbdf95ac15febdac0413ee1d07ad/CMakeLists.txt#L28
set(SPDLOG_USE_STD_FORMAT ON)
FetchContent_MakeAvailable(spdlog)

find_package(cpptrace REQUIRED)
find_package(glm REQUIRED)
find_package(GTest REQUIRED)
find_package(opengl_system REQUIRED)
find_package(SDL3 REQUIRED)
find_package(spdlog REQUIRED)

# polyfill for no cartesian_product in clang yet
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  find_package(range-v3 REQUIRED)
endif()

set_source_files_properties(src/glad.c PROPERTIES LANGUAGE CXX)

# main executable
add_executable(${PROJECT_NAME}
  src/active_keys.cpp
  src/event_loop.cpp
  src/glad.c
  src/gl_inspect.cpp
  src/grid.cpp
  src/key.cpp
  src/key_mod.cpp
  src/main.cpp
  src/opengl_debug_callback.cpp
  src/shader.cpp
  src/shader_program.cpp
  src/tessellation_settings.cpp
  src/tick_result.cpp
  src/vertices.cpp
  src/es/cpu_tessellation.cpp
  src/es/grid_points.cpp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_SWIZZLE)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_SWIZZLE_XYZW)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_MESSAGES=GLM_ENABLE)

# custom defines for src/
target_compile_definitions(${PROJECT_NAME} PRIVATE OPENGL_DEBUG)
# ref: https://stackoverflow.com/a/72330784
target_compile_definitions(${PROJECT_NAME} PRIVATE "DEBUG_BUILD=$<IF:$<CONFIG:Debug>,1,0>")

target_link_libraries(${PROJECT_NAME}
  PRIVATE cpptrace::cpptrace
  PRIVATE glm::glm
  PRIVATE opengl::opengl
  PRIVATE SDL3::SDL3
  PRIVATE spdlog::spdlog)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_include_directories(${PROJECT_NAME}
  PRIVATE ${3dgraph_SOURCE_DIR}/include
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_link_libraries(${PROJECT_NAME} PRIVATE range-v3::range-v3)
endif()

# opengl es 3.0 build
add_executable(${PROJECT_NAME}_es
  src/active_keys.cpp
  src/event_loop.cpp
  src/glad.c
  src/gl_inspect.cpp
  src/grid.cpp
  src/key.cpp
  src/key_mod.cpp
  src/main.cpp
  src/opengl_debug_callback.cpp
  src/shader.cpp
  src/shader_program.cpp
  src/tessellation_settings.cpp
  src/tick_result.cpp
  src/vertices.cpp
  src/es/cpu_tessellation.cpp
  src/es/grid_points.cpp
)

target_compile_definitions(${PROJECT_NAME}_es PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(${PROJECT_NAME}_es PRIVATE GLM_FORCE_SWIZZLE)
target_compile_definitions(${PROJECT_NAME}_es PRIVATE GLM_SWIZZLE_XYZW)
target_compile_definitions(${PROJECT_NAME}_es PRIVATE GLM_MESSAGES=GLM_ENABLE)

# custom defines for src/
target_compile_definitions(${PROJECT_NAME}_es PRIVATE OPENGL_DEBUG)
target_compile_definitions(${PROJECT_NAME}_es PRIVATE OPENGL_ES)

target_link_libraries(${PROJECT_NAME}_es
  PRIVATE cpptrace::cpptrace
  PRIVATE glm::glm
  PRIVATE opengl::opengl
  PRIVATE SDL3::SDL3
  PRIVATE spdlog::spdlog)
target_compile_features(${PROJECT_NAME}_es PRIVATE cxx_std_23)
target_include_directories(${PROJECT_NAME}_es
  PRIVATE ${3dgraph_SOURCE_DIR}/include
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_link_libraries(${PROJECT_NAME}_es PRIVATE range-v3::range-v3)
endif()

# tests
enable_testing()
set(TEST_WITH_COVERAGE "build and run test suite with code coverage" OFF)

add_executable(${PROJECT_NAME}_test
  src/active_keys.cpp
  src/key.cpp
  src/key_mod.cpp
  src/es/cpu_tessellation.cpp
  test/active_keys_test.cpp
  test/key_test.cpp
  test/key_mod_test.cpp
  test/es/cpu_tessellation_test.cpp
)

# NOTE: lcov does not like the output of the coverage files
# coverage is currently broken
if(TEST_WITH_COVERAGE)
  # ref: https://stackoverflow.com/a/78565049
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    target_compile_options(${PROJECT_NAME}_test INTERFACE --coverage -O0 -g)
    target_link_options(${PROJECT_NAME}_test INTERFACE --coverage)
  endif()
endif()

target_link_libraries(${PROJECT_NAME}_test 
  PRIVATE glm::glm
  # TODO: these are incorrect sourcing a globally installed gtest instead of
  # the one defined in the conan files
  PRIVATE GTest::gtest
  PRIVATE GTest::gtest_main
  PRIVATE SDL3::SDL3)
target_compile_features(${PROJECT_NAME}_test PRIVATE cxx_std_23)
target_include_directories(${PROJECT_NAME}_test
  PRIVATE ${3dgraph_SOURCE_DIR}/include
  PRIVATE ${3dgraph_SOURCE_DIR}/test/include
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_link_libraries(${PROJECT_NAME}_test PRIVATE range-v3::range-v3)
endif()

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  ADD_CUSTOM_TARGET(link_compile_commands_json ALL
                    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/compile_commands.json ${3dgraph_SOURCE_DIR}/compile_commands.json)
endif()
