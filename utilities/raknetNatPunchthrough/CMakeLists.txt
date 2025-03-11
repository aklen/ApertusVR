cmake_minimum_required(VERSION 3.10)
project(NatPunchThrough)

# RakNet könyvtárak
set(RAKNET_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/raknet/include/RakNet")
set(RAKNET_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../ApertusVR-build/raknet/build/Lib/LibStatic")

message(STATUS "RakNet include dir: ${RAKNET_INCLUDE_DIR}")
message(STATUS "RakNet library dir: ${RAKNET_LIB_DIR}")

set(SOURCES apeRaknetNatPunchthroughServer.cpp)

include_directories(${RAKNET_INCLUDE_DIR})
add_executable(apeRaknetNatPunchthroughServer ${SOURCES})

if(WIN32)
        target_link_libraries(apeRaknetNatPunchthroughServer ws2_32 ${RAKNET_LIB_DIR}/RakNetLibStatic.lib)
else()
        find_package(Threads REQUIRED)
        target_link_libraries(apeRaknetNatPunchthroughServer Threads::Threads ${RAKNET_LIB_DIR}/libRakNetLibStatic.a pthread)
endif()

set_property (TARGET apeRaknetNatPunchthroughServer PROPERTY FOLDER "Servers")
