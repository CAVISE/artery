macro(add_attack name sources)
    set(target capi_${name})
    add_artery_feature(${target} ${sources} ${ARGN})
    target_link_libraries(${target} PRIVATE capi)
endmacro()

add_attack(masquerade
    masquerade/Masquerade.h
    masquerade/Masquerade.cc
)

add_attack(flooding
    flooding/Flooding.h
    flooding/Flooding.cc
)

add_attack(replay
    replay/Replay.h
    replay/Replay.cc
)

add_attack(spamming
    spamming/Spamming.h
    spamming/Spamming.cc
)
