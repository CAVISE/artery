add_subdirectory(attacks)

add_opp_run(capi
    NED_FOLDERS
        ${CMAKE_SOURCE_DIR}/src/cavise
        ${CMAKE_CURRENT_SOURCE_DIR}
)
