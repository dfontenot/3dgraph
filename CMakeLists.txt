cmake_minimum_required(VERSION 3.17)
project(3dgraph
  VERSION 1.0
  DESCRIPTION "a very basic opengl render"
  LANGUAGES CXX)

include(FetchContent)
set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_SHALLOW 1
    SOURCE_SUBDIR glm
)

FetchContent_MakeAvailable(glm)

set_source_files_properties(src/glad.c PROPERTIES LANGUAGE CXX)
add_executable(${PROJECT_NAME} src/main.cpp src/glad.c)
#file(GLOB 3dgraph_SRC CONFIGURE_DEPENDS "src/*.cpp" "src/*.c")
#add_executable(${PROJECT_NAME} ${3dgraph_SRC})

#target_link_libraries(3dgraph PRIVATE ${SDL2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME}
  PRIVATE ${3dgraph_SOURCE_DIR}/include
  PRIVATE $<TARGET_PROPERTY:glm,INTERFACE_INCLUDE_DIRECTORIES>
  PRIVATE ${SDL2_INCLUDE_DIRS}
)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  ADD_CUSTOM_TARGET(link_compile_commands_json ALL
                    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/compile_commands.json ${3dgraph_SOURCE_DIR}/compile_commands.json)
endif()
