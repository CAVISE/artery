add_artery_feature(capi_masquerade
    masquerade/Masquerade.h
    masquerade/Masquerade.cc
)

target_link_libraries(capi_masquerade PRIVATE capi)

add_artery_feature(capi_flooding
    flooding/Flooding.h
    flooding/Flooding.cc
)

target_link_libraries(capi_flooding PRIVATE capi)

add_artery_feature(capi_replay
    replay/Replay.h
    replay/Replay.cc
)

target_link_libraries(capi_replay PRIVATE capi)

add_artery_feature(capi_spamming
    spamming/Spamming.h
    spamming/Spamming.cc
)

target_link_libraries(capi_spamming PRIVATE capi)

add_opp_run(capi
    NED_FOLDERS
        ${CMAKE_SOURCE_DIR}/src/cavise
        ${CMAKE_CURRENT_SOURCE_DIR}
)
