cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

project(LmiSolverInstalled LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies (all installed by the parent build) ----

include(../cmake/CPM.cmake)

# Find dependencies first — LmiSolverConfig.cmake references fmt::fmt and EllAlgo::EllAlgo
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(fmt REQUIRED)
find_package(EllAlgo REQUIRED)
find_package(LmiSolver REQUIRED)

set(SPECIFIC_LIBS fmt::fmt EllAlgo::EllAlgo Eigen3::Eigen)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE LmiSolver::LmiSolver ${SPECIFIC_LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
