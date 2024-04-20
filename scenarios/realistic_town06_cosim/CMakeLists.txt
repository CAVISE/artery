# Найдите и укажите необходимые библиотеки и пакеты
find_package(Protobuf REQUIRED)

# Генерация кода protobuf
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
                      Protobufs_structure/OpenCDA_message_structure.proto 
                      Protobufs_structure/Artery_message_structure.proto)

# Добавление исполняемого файла или библиотеки
add_artery_feature(ArteryManager
                    ArteryManager.cc
                    ${PROTO_SRCS} ${PROTO_HDRS})

# Указание директорий для поиска заголовочных файлов
target_include_directories(ArteryManager PRIVATE 
                           ${PROTOBUF_INCLUDE_DIRS} 
                           ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(ArteryManager PUBLIC MEMORY_BIN="Memory_BIN/")
target_compile_definitions(ArteryManager PUBLIC MEMORY_JSON="Messages_JSON/")
target_compile_definitions(ArteryManager PUBLIC OPENCDA_MESSAGE="Simulators_messages/OpenCDA_message.proto")
target_compile_definitions(ArteryManager PUBLIC INDICATE_LOGS="Logs/ArteryManager_indicate_logs.log")
target_compile_definitions(ArteryManager PUBLIC TRIGGER_LOGS="Logs/ArteryManager_trigger_logs.log") 
# Линковка необходимых библиотек
target_link_libraries(ArteryManager PRIVATE 
                      ${PROTOBUF_LIBRARIES})

# Добавление цели запуска в OMNeT++
add_opp_run(realistic_town06_cosim CONFIG omnetpp.ini)
