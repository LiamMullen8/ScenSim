cmake_minimum_required(VERSION 3.10)

project(ScenSim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(Boost 1.70 REQUIRED COMPONENTS system thread)
find_package(Protobuf REQUIRED)

# Protobuf compilation
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS src/proto/simulation.proto)

# Shared core library
add_library(ScenCore STATIC
    src/Session.cpp
    ${PROTO_SRCS}
)

# Executables
add_executable(ScenServer
    src/server_main.cpp
    src/Server.cpp
)

add_executable(ScenClient
    src/client_main.cpp
    src/Client.cpp
)

# Includes
target_include_directories(ScenCore PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(ScenServer PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(ScenClient PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Link libraries
target_link_libraries(ScenCore PUBLIC
    ${Protobuf_LIBRARIES}
    ${Boost_LIBRARIES}
)

target_link_libraries(ScenServer PRIVATE
    ScenCore
)

target_link_libraries(ScenClient PRIVATE
    ScenCore
)

# Compile settings
target_compile_features(ScenCore   PRIVATE cxx_std_17)
target_compile_features(ScenServer PRIVATE cxx_std_17)
target_compile_features(ScenClient PRIVATE cxx_std_17)

target_compile_options(ScenCore   PRIVATE -Wall -Wextra)
target_compile_options(ScenServer PRIVATE -Wall -Wextra)
target_compile_options(ScenClient PRIVATE -Wall -Wextra)


