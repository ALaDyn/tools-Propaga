cmake_minimum_required (VERSION 3.1)
project(tools-Propaga)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)

# make sure that the default is a RELEASE
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELEASE)
endif()

if(MSVC)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_USE_MATH_DEFINES)
endif()

set(JSONCONS_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/ext/jsoncons/include)
set(PHYSYCOM_UTILS_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/ext/utils)
find_package(Boost REQUIRED)
include_directories(${JSONCONS_INCLUDE_PATH} ${Boost_INCLUDE_DIR} ${PHYSYCOM_UTILS_INCLUDE_PATH})

add_executable(bunch-generator                ${CMAKE_SOURCE_DIR}/bunch-generator/bunch-generator.cpp ${CMAKE_SOURCE_DIR}/bunch-generator/NumberGenerator.cpp ${CMAKE_SOURCE_DIR}/bunch-generator/NumberGenerator.h)
add_executable(converter                      ${CMAKE_SOURCE_DIR}/converter/converter.cpp ${CMAKE_SOURCE_DIR}/converter/converter.h)
add_executable(revert-lines                   ${CMAKE_SOURCE_DIR}/revert-lines/revert-lines.cpp)
add_executable(scan-columns                   ${CMAKE_SOURCE_DIR}/scan-columns/scan-columns.cpp)
add_executable(scan-parameters                ${CMAKE_SOURCE_DIR}/scan-parameters/scan-parameters.cpp)
add_executable(scramble-lines                 ${CMAKE_SOURCE_DIR}/scramble-lines/scramble-lines.cpp)
add_executable(seleziona-particelle           ${CMAKE_SOURCE_DIR}/seleziona-particelle/seleziona.cpp ${CMAKE_SOURCE_DIR}/seleziona-particelle/seleziona.h)
add_executable(sum-columns                    ${CMAKE_SOURCE_DIR}/sum-columns/sum-columns.cpp)


install (TARGETS 
  bunch-generator
  converter
  revert-lines
  scan-columns
  scan-parameters
  scramble-lines
  seleziona-particelle
  sum-columns
  DESTINATION ${CMAKE_SOURCE_DIR}/bin/
)

