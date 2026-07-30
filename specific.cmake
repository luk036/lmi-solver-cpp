find_package(fmt CONFIG QUIET)
if(fmt_FOUND)
  message(STATUS "Found system fmt: ${fmt_DIR}")
  set(CPM_fmt_ADDED YES)
else()
  CPMAddPackage(
    NAME fmt
    GIT_TAG 12.1.0
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL YES"
  )
endif()

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
  GIT_TAG v1.6.8
  GITHUB_REPOSITORY luk036/ellalgo-cpp
  OPTIONS "INSTALL_ONLY YES"
)

set(SPECIFIC_LIBS fmt::fmt EllAlgo::EllAlgo)
