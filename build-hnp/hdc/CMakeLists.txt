cmake_minimum_required (VERSION 3.10)

project (hdc)

set (CMAKE_CXX_STANDARD 17)

add_definitions(-DHDC_HOST -DHARMONY_PROJECT -DHDC_SUPPORT_UART)

add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/hdc_hash_gen.h
    COMMAND python3 ${PROJECT_SOURCE_DIR}/hdc/scripts/hdc_hash_gen.py -f hdc_hash_gen.h -i hdc/ -o ./
    DEPENDS ${PROJECT_SOURCE_DIR}/hdc/scripts/hdc_hash_gen.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generating hdc_hash_gen.h using hdc_hash_gen.py"
)

add_custom_target(
    generate_hdc_hash_gen_h
    DEPENDS ${PROJECT_SOURCE_DIR}/hdc_hash_gen.h
)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../sysroot/include
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/securec/include
    ${PROJECT_SOURCE_DIR}/hdc/src/common
    ${PROJECT_SOURCE_DIR}/hdc/src/host
)

link_directories(
    ${PROJECT_SOURCE_DIR}/../../sysroot/lib
)

file(GLOB SRC_FILES
    ${PROJECT_SOURCE_DIR}/securec/src/*.c
    ${PROJECT_SOURCE_DIR}/hdc/src/common/*.cpp
    ${PROJECT_SOURCE_DIR}/hdc/src/host/*.cpp
)

add_executable(hdc ${SRC_FILES})
add_dependencies(hdc generate_hdc_hash_gen_h)
target_link_libraries(hdc libuv.so)
target_link_libraries(hdc libusb-1.0.so)
target_link_libraries(hdc libssl.so)
target_link_libraries(hdc libcrypto.so)
target_link_libraries(hdc liblz4.so)
