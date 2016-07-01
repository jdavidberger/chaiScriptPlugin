cmake_minimum_required(VERSION 2.8.11)

project(dbgPlugin CXX C)

set(x64dbgfolder C:/tools/x64dbg )

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${x64dbgfolder}/release/x32/plugins)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${x64dbgfolder}/release/x32/plugins)

include_directories( ${x64dbgfolder} )
LINK_DIRECTORIES( ${x64dbgfolder}/pluginsdk )


add_library(dbgPlugin SHARED
    main.cc
)

target_link_libraries(dbgPlugin x32dbg x32bridge )
set_target_properties(dbgPlugin PROPERTIES SUFFIX ".dp32")
