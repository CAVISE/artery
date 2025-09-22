####################
# CAVISE SUBMODULE #
####################

add_artery_feature(cavise
    Init.cc 
    Init.h
    application/CosimService.cc
    application/CosimService.h
    comms/CommunicationManager.h
    comms/SingletonHolder.h
)

add_protobuf(capi-protos PROTOS messages/capi.proto)

add_opp_message(capi-opp-message MESSAGE application/Opencda.msg MSG_PREFIX cavise_msgs)

target_link_libraries(cavise PUBLIC capi-protos PkgConfig::ZEROMQ)

target_include_directories(cavise PUBLIC ${PROJECT_SOURCE_DIR}/src)
