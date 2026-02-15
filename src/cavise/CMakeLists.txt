####################
# CAVISE SUBMODULE #
####################

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

target_link_libraries(capi PUBLIC cppzmq)

add_opp_message(
    cosim-message
    MESSAGE application/CosimMessage.msg
    MSG_PREFIX cavise_msgs
)

target_link_libraries(capi PUBLIC cosim-message)

add_subdirectory(messages)
