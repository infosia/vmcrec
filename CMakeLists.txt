cmake_minimum_required(VERSION 2.8.12)
PROJECT(vmcrec CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(third_party/flatbuffers EXCLUDE_FROM_ALL)
add_subdirectory(third_party/oscpack EXCLUDE_FROM_ALL)

set(vmcrec_FILES
  src/main.cpp
  src/VMCPacketListener.cpp
  include/VMCPacketListener.hpp
)

set (VMCREC_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cgltf
)

set(EXE_NAME vmcrec)
add_executable(${EXE_NAME} ${vmcrec_FILES})
set_property(TARGET ${EXE_NAME} PROPERTY CXX_STANDARD 11)

target_include_directories(${EXE_NAME} PRIVATE ${VMCREC_INCLUDES})

target_link_libraries(${EXE_NAME} PRIVATE flatbuffers)
target_link_libraries(${EXE_NAME} PRIVATE oscpack Ws2_32 winmm)

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
