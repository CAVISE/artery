set(PROTO_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PROTO_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_library(protos
    OBJECT
        ${PROTO_SOURCE_DIR}/capi/capi.proto
        ${PROTO_SOURCE_DIR}/capi/artery.proto
        ${PROTO_SOURCE_DIR}/capi/opencda.proto
        ${PROTO_SOURCE_DIR}/capi/entity.proto
)

protobuf_generate(
    TARGET protos
    IMPORT_DIRS ${PROTO_SOURCE_DIR}
    PROTOC_OUT_DIR ${PROTO_BINARY_DIR}
    LANGUAGE cpp
)

artery_mark_as_codegen(protos)

target_include_directories(protos PUBLIC
    ${PROTO_BINARY_DIR}
    ${PROTO_BINARY_DIR}/capi
)

target_link_libraries(protos PUBLIC protobuf::libprotobuf)
