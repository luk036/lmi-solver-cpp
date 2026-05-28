cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

option(CPM_USE_LOCAL_PACKAGES "Use Local package" TRUE)
option(INSTALL_ONLY "Enable for installation only" OFF)

project(
  LmiSolver
  VERSION 1.0.0
  LANGUAGES CXX
)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed.")
endif()

include(cmake/tools.cmake)
include(cmake/CPM.cmake)

CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(specific.cmake)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

add_library(${PROJECT_NAME} ${headers} ${sources})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/wd4267>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/wd4244>")
target_compile_options(
  ${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,Clang>:-Wno-sign-conversion>"
)
target_compile_options(
  ${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-sign-conversion>"
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${SPECIFIC_LIBS})

target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
  VERSION_HEADER "${VERSION_HEADER_LOCATION}"
  COMPATIBILITY SameMajorVersion
)

if(NOT INSTALL_ONLY)
  add_subdirectory(test)
endif()
