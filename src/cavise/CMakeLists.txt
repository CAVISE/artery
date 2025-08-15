####################
# CAVISE SUBMODULE #
####################

# This file is entrypoint for common CAVISE tools & enhancements to Artery.
cmake_minimum_required(VERSION 3.12)

project(cavise-artery VERSION 0.1.0 LANGUAGES CXX)

add_artery_feature(capi
    Init.h
    Init.cc
    CAPIManager.h
    CAPIManager.cc
    application/CosimService.h
    application/CosimService.cc
)

set(PROTO_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/messages)
set(PROTO_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protos)
set(PROTO_GENERATOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protos/cavise)

set(PROTOS 
    ${PROTO_SOURCE_DIR}/capi.proto
    ${PROTO_SOURCE_DIR}/artery.proto
    ${PROTO_SOURCE_DIR}/opencda.proto
    ${PROTO_SOURCE_DIR}/entity.proto
)

add_library(protos OBJECT ${PROTOS})
protobuf_generate(
    TARGET protos 
    IMPORT_DIRS ${PROTO_SOURCE_DIR} 
    PROTOC_OUT_DIR ${PROTO_GENERATOR_DIR}
    LANGUAGE cpp
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(protos PUBLIC "${CMAKE_CXX_FLAGS}" "-Wall" "-Wextra" "-fPIC")
    target_compile_options(capi PUBLIC "${CMAKE_CXX_FLAGS}" "-Wall" "-Wextra")
else()
    message(WARNING "Make sure PIC (Position Independent Code) is supported by default by your compiler!")
endif()

# I think version 5.xx.x renamed protoc component, so it is necessary to link this with
# protobuf::libprotoc 
target_link_libraries(protos PUBLIC protobuf::libprotoc protobuf::libprotobuf)

target_include_directories(protos PUBLIC 
    ${PROTO_BINARY_DIR}
    # include libprotobuf headers to override system libprotobuf if present
    ${protobuf_INCLUDE_DIRS}
)

generate_opp_message(application/Opencda.msg TARGET capi DIRECTORY cavise_msgs)

target_link_libraries(
    capi PUBLIC
    protos
    cppzmq 
    plog::plog
)

target_include_directories(
    capi PUBLIC 
    ${ROOT_DIR} 
    ${plog_INCLUDE_DIRS}
    ${cppzmq_INCLUDE_DIRS}
)
