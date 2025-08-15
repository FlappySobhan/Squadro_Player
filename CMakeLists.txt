# CMake minimum version requirement
cmake_minimum_required(VERSION 3.16)

# Project name and language
project(SquadroAI CXX)

# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable and its source files
# NOTE: Socket.cpp has been removed
add_executable(squadro_bot
    src/main.cpp
    src/Board.cpp
    src/MinimaxAI.cpp
    src/GameController.cpp
)

# Include the 'include' directory for header files
target_include_directories(squadro_bot PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link networking and threading libraries based on the operating system
if (WIN32)
    # For Windows, link the Winsock and threading libraries
    target_link_libraries(squadro_bot ws2_32 wsock32)
else()
    # For Linux/macOS, link pthreads
    target_link_libraries(squadro_bot pthread)
endif()

# Optional: Add compiler flags for warnings
if (MSVC)
    target_compile_options(squadro_bot PRIVATE /W4)
else()
    target_compile_options(squadro_bot PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Installation instructions (optional)
install(TARGETS squadro_bot DESTINATION bin)

message(STATUS "CMake configuration complete. Use 'cmake --build .' to compile.")
