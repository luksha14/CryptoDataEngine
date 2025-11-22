cmake_minimum_required(VERSION 3.10)
project(DataProcessingEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_executable(data_engine
    src/main.cpp
    src/DataIngestor.cpp
    src/Persistence.cpp
    src/ProcessingThread.cpp
    src/TickerData.cpp
    src/sqlite3.c 
)

# Osiguravamo da se koriste headeri iz 'include' foldera
target_include_directories(data_engine PUBLIC 
    include
)

# Povezivanje sistemske pthread biblioteke
target_link_libraries(data_engine 
    pthread
    sqlite3
    ws2_32
)