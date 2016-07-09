cmake_minimum_required(VERSION 2.8.11)

project(chaiScriptPlugin CXX C)

if(NOT X64DBGFOLDER)
    if($ENV{X64DBGFOLDER})
        set(X64DBGFOLDER $ENV{X64DBGFOLDER} )
    else()
        set(X64DBGFOLDER .. )
    endif()
endif()

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    set(SUFFIX "d")
else()
    set(SUFFIX "")
ENDIF()

if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    set(ARCH "32")
    set(ARCH_NAME "86")
else()
    set(ARCH "64")
    set(ARCH_NAME "64")
endif()
message("Building with suffix ${SUFFIX}(${CMAKE_BUILD_TYPE}), for x${ARCH}")
# Figure out if the folder given is a release or not
if(EXISTS ${X64DBGFOLDER}/bin)
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${X64DBGFOLDER}/bin/x${ARCH}${SUFFIX}/plugins)
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${X64DBGFOLDER}/bin/x${ARCH}${SUFFIX}/plugins)
    include_directories( ${X64DBGFOLDER}/release )

    set(PLUGINSDK_DIR ${X64DBGFOLDER}/release/pluginsdk )
    LINK_DIRECTORIES( ${X64DBGFOLDER}/bin/x${ARCH}${SUFFIX})
else()
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${X64DBGFOLDER}/release/x${ARCH}${SUFFIX}/plugins)
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${X64DBGFOLDER}/release/x${ARCH}${SUFFIX}/plugins)
    include_directories( ${X64DBGFOLDER} )

    set(PLUGINSDK_DIR ${X64DBGFOLDER}/pluginsdk )
    LINK_DIRECTORIES( ${X64DBGFOLDER}/pluginsdk )
endif()

LINK_DIRECTORIES(${PLUGINSDK_DIR}/jansson)

include_directories( . ChaiScript/include)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)
find_package(Qt5Core)

add_definitions(/Zm250)
add_definitions(/bigobj)

add_library(chaiScriptPlugin SHARED
    Std.cc
    Std.h
    pluginmain.cc
    dbgops.h    
)

set_source_files_properties(Std.cc PROPERTIES COMPILE_FLAGS "/YcStd.h")
set_source_files_properties(pluginmain.cc   PROPERTIES COMPILE_FLAGS "/YuStd.h")

target_link_libraries(chaiScriptPlugin
        x${ARCH}dbg
        x${ARCH}bridge
        Qt5::Core Qt5::Widgets
        jansson_x${ARCH_NAME}
    )
set_target_properties(chaiScriptPlugin PROPERTIES SUFFIX ".dp${ARCH}")
