cmake_minimum_required(VERSION 2.8.11)

project(chaiScriptPlugin CXX C)

if(NOT X64DBGFOLDER)
    if($ENV{X64DBGFOLDER})
        set(X64DBGFOLDER $ENV{X64DBGFOLDER} )
    else()
        set(X64DBGFOLDER .. )
    endif()
endif()

if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    set(ARCH "32")
else()
    set(ARCH "64")
endif()

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${X64DBGFOLDER}/release/x${ARCH}/plugins)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${X64DBGFOLDER}/release/x${ARCH}/plugins)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")

find_package(Qt5Widgets)
find_package(Qt5Core)

include_directories( . ${X64DBGFOLDER} ChaiScript/include)
LINK_DIRECTORIES( ${X64DBGFOLDER}/pluginsdk )

add_library(chaiScriptPlugin SHARED
    pluginmain.cc
    dbgops.h
    chaiInterops.h
)

target_link_libraries(chaiScriptPlugin x${ARCH}dbg x${ARCH}bridge Qt5::Core Qt5::Widgets  )
set_target_properties(chaiScriptPlugin PROPERTIES SUFFIX ".dp${ARCH}")
