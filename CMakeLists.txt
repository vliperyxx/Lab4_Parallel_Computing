cmake_minimum_required(VERSION 3.30)
project(Lab4_Parallel_Computing)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES_COMMON
        MatrixOperations/MatrixOperations.cpp
        "CommandHandler/CommandHandler.cpp"
)

set(HEADERS
        MatrixOperations/MatrixOperations.h
        "CommandHandler/CommandHandler.h"
)

add_executable(Server Server.cpp ${SOURCES_COMMON} ${HEADERS})
add_executable(Client Client.cpp ${SOURCES_COMMON} ${HEADERS})

target_link_libraries(Server ws2_32)
target_link_libraries(Client ws2_32)