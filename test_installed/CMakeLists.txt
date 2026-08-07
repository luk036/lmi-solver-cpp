cmake_minimum_required(VERSION 3.14)
project(LmiSolverInstalledTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find dependencies first - the installed config references fmt and EllAlgo; the public headers
# use Eigen.
find_package(fmt REQUIRED)
find_package(EllAlgo REQUIRED)
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(LmiSolver REQUIRED)

add_executable(lmisolver_installed_test main.cpp)
target_link_libraries(lmisolver_installed_test PRIVATE LmiSolver::LmiSolver fmt::fmt Eigen3::Eigen)

enable_testing()
add_test(NAME lmisolver_installed_test COMMAND lmisolver_installed_test)
