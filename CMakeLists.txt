cmake_minimum_required(VERSION 3.10)
project(SendFile_CommonAPI_Demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CommonAPI REQUIRED)
find_package(CommonAPI-SomeIP REQUIRED)
find_package(vsomeip3 REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/Src
    ${CMAKE_CURRENT_LIST_DIR}/fidl/src-gen
)

# ============================================================
# GENERATED CODE
# ============================================================
set(GENERATED_SOMEIP_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/fidl/src-gen/v1/ayman/examples/SendFileSomeIPDeployment.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fidl/src-gen/v1/ayman/examples/SendFileSomeIPProxy.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fidl/src-gen/v1/ayman/examples/SendFileSomeIPStubAdapter.cpp
)

# ============================================================
# SHARED LIBRARY FOR SOMEIP BINDINGS
# ============================================================
add_library(SendFile-someip SHARED
    ${GENERATED_SOMEIP_SOURCES}
)

target_link_libraries(SendFile-someip
    CommonAPI
    CommonAPI-SomeIP
    vsomeip3
)

# ============================================================
# SERVER APPLICATION
# ============================================================
add_executable(SendFileServer
    Src/Server.cpp
)

target_link_libraries(SendFileServer
    CommonAPI
    CommonAPI-SomeIP
    vsomeip3
    SendFile-someip
)

# ============================================================
# CLIENT APPLICATION
# ============================================================
add_executable(SendFileClient
    Src/Client.cpp
)

target_link_libraries(SendFileClient
    CommonAPI
    CommonAPI-SomeIP
    vsomeip3
    SendFile-someip
)