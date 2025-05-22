cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)

project(done)
add_executable(main main.cc)

add_library(done done.cc)
target_link_libraries(main PUBLIC done)
target_include_directories(done PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(done PUBLIC ${CMAKE_SOURCE_DIR}/v8/libv8_monolith.a)
target_compile_definitions(done PUBLIC
    V8_TYPED_ARRAY_MAX_SIZE_IN_HEAP=0
    V8_COMPRESS_POINTERS
    V8_INTL_SUPPORT=1
    ENABLE_HUGEPAGE
)

target_include_directories(done PUBLIC ${CMAKE_SOURCE_DIR}/v8/include)

add_library(console src/console.cc)
target_link_libraries(done PUBLIC console)
target_include_directories(console PUBLIC v8/include)

add_library(fetch src/fetch.cc)
target_link_libraries(done PUBLIC fetch)
target_include_directories(fetch PUBLIC v8/include)

add_library(module src/module.cc)
target_link_libraries(done PUBLIC module)
target_include_directories(module PUBLIC v8/include)

add_library(process src/process.cc)
target_link_libraries(done PUBLIC process)
target_include_directories(process PUBLIC v8/include)

add_library(syscall-wrapper src/syscall-wrapper.cc)
target_link_libraries(done PUBLIC syscall-wrapper)
target_include_directories(syscall-wrapper PUBLIC v8/include)

add_library(text-decoder src/text-decoder.cc)
target_link_libraries(done PUBLIC text-decoder)
target_include_directories(text-decoder PUBLIC v8/include)

add_library(text-encoder src/text-encoder.cc)
target_link_libraries(done PUBLIC text-encoder)
target_include_directories(text-encoder PUBLIC v8/include)

add_library(utils src/internal/util.cc)
target_link_libraries(done PUBLIC utils)
target_include_directories(utils PUBLIC v8/include)

target_include_directories(done PUBLIC src)


file(COPY ${CMAKE_SOURCE_DIR}/lib DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/test DESTINATION ${CMAKE_BINARY_DIR})
