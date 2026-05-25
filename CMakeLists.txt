cmake_minimum_required(VERSION 3.13)
project(deepANS VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
        source/deepANS.cpp
        source/StaticCoder.cpp
        source/Lib/Utils/global_logger.cpp
        source/Lib/Utils/Bitstream.h
        source/Lib/EncLib/CABACEncoder.cpp
        source/Lib/EncLib/BinEncoder_simple.cpp
        source/Lib/DecLib/CABACDecoder.cpp
        source/Lib/DecLib/BinDecoder.cpp
        source/Lib/CommonLib/ContextModel.cpp
        source/Lib/CommonLib/ContextModeler.cpp
        source/Lib/CommonLib/TabledANS.h
        source/Lib/CommonLib/UniformRangeANS.h
        source/Lib/Test/test_ans_coding.cpp
)

add_executable(deepANS ${SOURCES})
target_include_directories(deepANS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source) # Enable use headers without ../ (like it)
target_compile_options(deepANS PRIVATE -O2 -Wall -Wextra)
target_link_options(deepANS PRIVATE -static -static-libgcc -static-libstdc++)
