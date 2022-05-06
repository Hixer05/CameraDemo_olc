cmake_minimum_required(VERSION 3.0.0)
project(olcPGE_demos VERSION 0.1.0)

add_executable(olcPGE_demos gravity_refactored.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

target_include_directories(${PROJECT_NAME} PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

include(CheckCXXCompilerFlag)

function(add_cxx_compiler_flags var)
  foreach(flag ${ARGN})
    string(REGEX REPLACE "[^a-zA-Z0-9]+" "_" flag_var "CXXFLAG_${flag}")
    check_cxx_compiler_flag("${flag}" ${flag_var})
    if(${flag_var})
      set(${var} "${${var}} ${flag}")
    endif()
  endforeach()
  set(${var} "${${var}}" PARENT_SCOPE)
endfunction()

add_cxx_compiler_flags(CMAKE_CXX_FLAGS
    -lX11
    -lGL 
    -lpthread 
    -lpng 
    -lstdc++fs 
    -std=c++17
)