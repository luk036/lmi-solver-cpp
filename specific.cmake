CPMAddPackage(
  NAME fmt
  GIT_TAG 12.1.0
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL YES"
)

CPMAddPackage(
  NAME Eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 5.0.1
  DOWNLOAD_ONLY YES
)

if(Eigen_ADDED)
  message(STATUS "Found Eigen: ${Eigen_SOURCE_DIR}")
  include_directories(${Eigen_SOURCE_DIR})
endif()

# EllAlgo needed for integration tests (cutting_plane_optim, Ell, EllStable)
CPMAddPackage(
  NAME EllAlgo
  GIT_TAG 1.6.5
  GITHUB_REPOSITORY luk036/ellalgo-cpp
  OPTIONS "INSTALL_ONLY YES"
)

set(SPECIFIC_LIBS fmt::fmt EllAlgo::EllAlgo)
