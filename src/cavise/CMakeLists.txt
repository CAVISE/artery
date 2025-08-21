####################
# CAVISE SUBMODULE #
####################

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_artery_feature(capi
    capi/Core.h
    capi/Core.cc
    capi/Listener.h
    capi/Listener.cc
    capi/ConnectionHandler.h
    capi/ConnectionHandler.cc
    application/CosimService.h
    application/CosimService.cc
)

set(PROTO_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/messages)
set(PROTO_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protos)
set(PROTO_GENERATOR_DIR ${PROTO_BINARY_DIR}/cavise)

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

target_include_directories(protos PUBLIC ${PROTO_BINARY_DIR})

generate_opp_message(application/CosimMessage.msg TARGET capi DIRECTORY cavise_msgs)

target_link_libraries(capi PUBLIC protos cppzmq)

target_include_directories(capi PUBLIC ${ROOT_DIR})
