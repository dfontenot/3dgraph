cmake_minimum_required(VERSION 3.23)
project(3dgraph
  VERSION 1.0
  DESCRIPTION "a very basic opengl render"
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)

# workaround for https://github.com/conan-io/conan-center-index/issues/25185
FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_SHALLOW 1
  GIT_TAG v1.15.3
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

set_source_files_properties(src/glad.c PROPERTIES LANGUAGE CXX)

# main executable
add_executable(${PROJECT_NAME}
  src/active_keys.cpp
  src/event_loop.cpp
  src/glad.c
  src/gl_inspect.cpp
  src/key.cpp
  src/main.cpp
  src/opengl_debug_callback.cpp
  src/shader.cpp
  src/shader_program.cpp
  src/tessellation_settings.cpp
  src/es/cpu_tessellation.cpp
)

target_precompile_headers(${PROJECT_NAME} PRIVATE "${3dgraph_SOURCE_DIR}/include/formatters.hpp")

target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_SWIZZLE)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_SWIZZLE_XYZW)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_MESSAGES=GLM_ENABLE)

# custom defines for src/
target_compile_definitions(${PROJECT_NAME} PRIVATE OPENGL_DEBUG)

target_link_libraries(${PROJECT_NAME}
  PRIVATE cpptrace::cpptrace
  PRIVATE glm::glm
  PRIVATE opengl::opengl
  PRIVATE SDL3::SDL3
  PRIVATE spdlog::spdlog)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME}
  PRIVATE ${3dgraph_SOURCE_DIR}/include
)

# tests
enable_testing()
set(TEST_WITH_COVERAGE "build and run test suite with code coverage" OFF)

add_executable(${PROJECT_NAME}_test
  src/active_keys.cpp
  src/key.cpp
  src/es/cpu_tessellation.cpp
  test/active_keys_test.cpp
  test/key_test.cpp
  test/es/cpu_tessellation_test.cpp
)

target_precompile_headers(${PROJECT_NAME}_test PRIVATE "${3dgraph_SOURCE_DIR}/include/formatters.hpp")

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
target_compile_features(${PROJECT_NAME}_test PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME}_test
  PRIVATE ${3dgraph_SOURCE_DIR}/include
  PRIVATE ${3dgraph_SOURCE_DIR}/test/include
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  ADD_CUSTOM_TARGET(link_compile_commands_json ALL
                    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/compile_commands.json ${3dgraph_SOURCE_DIR}/compile_commands.json)
endif()
