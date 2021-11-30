cpmaddpackage(
  NAME
  fmt
  GIT_TAG
  7.1.3
  GITHUB_REPOSITORY
  fmtlib/fmt
  OPTIONS
  "FMT_INSTALL YES" # create an installable target
)

cpmaddpackage("gh:xtensor-stack/xtl#0.6.23")
if(xtl_ADDED)
  message(STATUS "Found xtl: ${xtl_SOURCE_DIR}")
  include_directories(${xtl_SOURCE_DIR}/include)
endif(xtl_ADDED)

cpmaddpackage("gh:xtensor-stack/xtensor#0.22.0")
if(xtensor_ADDED)
  message(STATUS "Found xtensor: ${xtensor_SOURCE_DIR}")
  include_directories(${xtensor_SOURCE_DIR}/include)
endif(xtensor_ADDED)

cpmaddpackage(
  NAME
  EllAlgo
  GIT_TAG
  1.1
  GITHUB_REPOSITORY
  luk036/ellalgo-cpp
  OPTIONS
  "INSTALL_ONLY YES" # create an installable target
)

cpmaddpackage("gh:microsoft/GSL@3.1.0")

set(SPECIFIC_LIBS EllAlgo::EllAlgo fmt::fmt GSL)
