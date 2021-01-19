cmake_minimum_required( VERSION 2.8 )
PROJECT(oscpack CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    set(IpSystemTypePath ip/win32)
    set(LIBS ${LIBS} Ws2_32 winmm)
else()
    set(IpSystemTypePath ip/posix)
endif()

add_library(oscpack STATIC
    ip/IpEndpointName.h
    ip/IpEndpointName.cpp
    ip/NetworkingUtils.h
    ${IpSystemTypePath}/NetworkingUtils.cpp
    ip/UdpSocket.h
    ${IpSystemTypePath}/UdpSocket.cpp
    ip/PacketListener.h
    ip/TimerListener.h
    osc/OscTypes.h
    osc/OscTypes.cpp 
    osc/OscHostEndianness.h
    osc/OscException.h
    osc/OscPacketListener.h
    osc/MessageMappingOscPacketListener.h
    osc/OscReceivedElements.h
    osc/OscReceivedElements.cpp
    osc/OscPrintReceivedElements.h
    osc/OscPrintReceivedElements.cpp
    osc/OscOutboundPacketStream.h
    osc/OscOutboundPacketStream.cpp
)

target_include_directories(oscpack INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-long-long -pedantic")
endif()
