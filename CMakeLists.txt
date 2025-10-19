cmake_minimum_required(VERSION 3.10)

project(ScenSim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost
find_package(Boost 1.70 REQUIRED COMPONENTS system thread)

if(Boost_FOUND)
    message(STATUS "Boost found, version ${Boost_VERSION}")
else()
    message(FATAL_ERROR "Boost not found.")
endif()

add_executable(ScenServer
    src/server_main.cpp
    src/Server.cpp
    src/Session.cpp
)

add_executable(ScenClient
    src/client_main.cpp
    src/Client.cpp
)

set(ALL_TARGETS ScenServer ScenClient)

target_include_directories(ScenServer PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src
)

target_include_directories(ScenClient PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(ScenServer PRIVATE
    ${Boost_LIBRARIES}
)

target_link_libraries(ScenClient PRIVATE
    ${Boost_LIBRARIES}
)

target_compile_features(ScenServer PRIVATE cxx_std_17)
target_compile_features(ScenServer PRIVATE cxx_std_17)

target_compile_options(ScenClient PRIVATE -Wall -Wextra)
target_compile_options(ScenClient PRIVATE -Wall -Wextra)


