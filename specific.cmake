CPMAddPackage(
  NAME fmt
  GIT_TAG 12.1.0
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL YES"
)

CPMAddPackage(
  NAME Eigen
  GIT_TAG 5.0.1
  GITHUB_REPOSITORY eigenteam/eigen-git-mirror
)

if(Eigen_ADDED)
  message(STATUS "Found Eigen: ${Eigen_SOURCE_DIR}")
  include_directories(${Eigen_SOURCE_DIR})
endif()

set(SPECIFIC_LIBS fmt::fmt)
