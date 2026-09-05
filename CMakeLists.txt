cmake_minimum_required(VERSION 3.13)
project(StaticANS VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
        source/StaticANS.cpp
        source/Lib/Utils/global_logger.cpp
        source/Lib/Utils/Bitstream.h
        source/Lib/EncLib/CABACEncoder.cpp
        source/Lib/EncLib/BinEncoder_simple.cpp
        source/Lib/EncLib/ANSEncoder.cpp
        source/Lib/DecLib/CABACDecoder.cpp
        source/Lib/DecLib/BinDecoder.cpp
        source/Lib/DecLib/ANSDecoder.cpp
        source/Lib/CommonLib/Context.cpp
        source/Lib/CommonLib/ContextModel.cpp
        source/Lib/CommonLib/ContextModeler.cpp
        source/Lib/CommonLib/TabledANS.h
        source/Lib/CommonLib/BinaryEquiprobableANS.h
        source/Entrypoint.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")
add_executable(StaticANS ${SOURCES})
target_include_directories(StaticANS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source) # Enable use headers without ../ (like it)
target_compile_options(StaticANS PRIVATE -Wall -Wextra)
target_link_options(StaticANS PRIVATE -static -static-libgcc -static-libstdc++)

if(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g3 -fno-omit-frame-pointer -DNDEBUG")
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()