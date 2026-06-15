option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)

CPMAddPackage("gh:doctest/doctest#v2.4.11")
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME}Tests ${sources})
target_link_libraries(
  ${PROJECT_NAME}Tests doctest::doctest ${PROJECT_NAME}::${PROJECT_NAME} EllAlgo::EllAlgo
  ${SPECIFIC_LIBS}
)
set_target_properties(${PROJECT_NAME}Tests PROPERTIES CXX_STANDARD 20)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(
    ${PROJECT_NAME}
    PUBLIC -Wall
           -Wextra
           -Wconversion
           -Wno-parentheses
           -Wno-unused-but-set-parameter
           -Wno-sign-conversion
           -Wno-deprecated-literal-operator
  )
elseif(MSVC)
  target_compile_options(
    ${PROJECT_NAME}
    PUBLIC /utf-8 /W4
           /WX
           /wd5285
           /wd4267
           /wd4244
  )
  # target_compile_definitions(${PROJECT_NAME}Tests PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
endif()

enable_testing()

string(TOLOWER ${PROJECT_NAME} PROJECT_FILE_NAME)
add_test(NAME ${PROJECT_FILE_NAME}Tests COMMAND ${PROJECT_NAME}Tests)

if(ENABLE_TEST_COVERAGE)
  target_compile_options(${PROJECT_NAME} PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(${PROJECT_NAME} PUBLIC -fprofile-arcs -ftest-coverage)
endif()
