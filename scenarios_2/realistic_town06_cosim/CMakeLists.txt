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

# Линковка необходимых библиотек
target_link_libraries(ArteryManager PRIVATE 
                      ${PROTOBUF_LIBRARIES})

# Добавление цели запуска в OMNeT++
add_opp_run(realistic_town06_cosim CONFIG omnetpp.ini)
