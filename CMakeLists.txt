cmake_minimum_required(VERSION 3.15)
project(Jlang LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

add_executable(Jlang ${SRC_FILES} ${RUNTIME_FILE})

source_group(TREE "${CMAKE_SOURCE_DIR}/src" PREFIX "src" FILES ${SRC_FILES})
source_group("root" FILES ${RUNTIME_FILE})

target_include_directories(Jlang PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

target_include_directories(Jlang PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_directories(Jlang PRIVATE ${LLVM_LIBRARY_DIRS})
target_compile_definitions(Jlang PRIVATE ${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(LLVM_LIBS
    Core
    Support
    ExecutionEngine
    native
)

target_link_libraries(Jlang PRIVATE ${LLVM_LIBS})
