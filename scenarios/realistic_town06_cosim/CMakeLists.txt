cmake_minimum_required(VERSION 3.4)

# Найдите и укажите необходимые библиотеки и пакеты
find_package(Protobuf REQUIRED VERSION 3.18.1 CONFIG)
find_package(ZeroMQ REQUIRED VERSION 4.3.2 CONFIG)

# Генерация кода protobuf
set(PROTO_SOURCE_DIR messages)

set(PROTOS )
list(APPEND PROTOS ${PROTO_SOURCE_DIR}/artery.proto)
list(APPEND PROTOS ${PROTO_SOURCE_DIR}/opencda.proto)

set(PROTO_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/cavise/messages)

add_library(protos OBJECT ${PROTOS})

file(MAKE_DIRECTORY ${PROTO_BINARY_DIR})
target_include_directories(protos PUBLIC 
    "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>"
    # include libprotobuf headers to override system libprotobuf if present
    ${protobuf_INCLUDE_DIRS}
)

target_link_libraries(protos PUBLIC protobuf::libprotobuf)

protobuf_generate(
    TARGET protos 
    IMPORT_DIRS ${PROTO_SOURCE_DIR} 
    PROTOC_OUT_DIR ${PROTO_BINARY_DIR}
    GENERATE_EXTENSIONS python cpp
)

# Добавление исполняемого файла или библиотеки
add_artery_feature(ArteryManager
                    ArteryManager.cc
                    ${PROTO_SRCS} ${PROTO_HDRS})

# Указание директорий для поиска заголовочных файлов
target_include_directories(ArteryManager PUBLIC 
                           ${PROTOBUF_INCLUDE_DIRS} 
                           ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(ArteryManager PUBLIC MEMORY_BIN="Memory_BIN/")
target_compile_definitions(ArteryManager PUBLIC MEMORY_JSON="Messages_JSON/")
target_compile_definitions(ArteryManager PUBLIC OPENCDA_MESSAGE="Simulators_messages/OpenCDA_message.proto")
target_compile_definitions(ArteryManager PUBLIC INDICATE_LOGS="Logs/ArteryManager_indicate_logs.log")
target_compile_definitions(ArteryManager PUBLIC TRIGGER_LOGS="Logs/ArteryManager_trigger_logs.log") 

# Линковка необходимых библиотек
target_link_libraries(ArteryManager PRIVATE protobuf::libprotobuf libzmq-static)

# Добавление цели запуска в OMNeT++
add_opp_run(realistic_town06_cosim CONFIG omnetpp.ini)
