add_artery_feature(capi_manager_dummy
    DummyDealer.cc
    MessageRegistry.cc
)
target_link_libraries(capi_manager_dummy PRIVATE capi)

add_opp_run(
    capi_manager
    CONFIG omnetpp.ini
    NED_FOLDERS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/cavise
)
