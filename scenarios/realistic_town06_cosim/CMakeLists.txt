find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS TransfusionMsg.proto)

add_artery_feature(Transfusion
    TransfusionLoopback.cc
    TransfusionService.cc
    ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(Transfusion PRIVATE ${PROTOBUF_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Transfusion PRIVATE ${PROTOBUF_LIBRARIES})

# generat_opp_message
add_opp_run(realistic_town06_cosim CONFIG omnetpp.ini)
