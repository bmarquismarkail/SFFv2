cmake_minimum_required(VERSION 3.14)
project(sff2 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(sff2 STATIC src/sff2.cpp)
add_library(sff2::sff2 ALIAS sff2)
target_include_directories(sff2 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(sff2 PUBLIC cxx_std_11)

option(SFF2_BUILD_EXAMPLE "Build the command-line example" ON)
option(SFF2_BUILD_TESTS "Build the tests" ON)

if(SFF2_BUILD_EXAMPLE)
    add_executable(sff2_example examples/example.cpp)
    target_link_libraries(sff2_example PRIVATE sff2::sff2)
endif()

if(SFF2_BUILD_TESTS)
    enable_testing()
    add_executable(sff2_test tests/sff2_test.cpp)
    target_link_libraries(sff2_test PRIVATE sff2::sff2)
    add_test(NAME sff2_test COMMAND sff2_test)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
install(TARGETS sff2 EXPORT sff2Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES include/sff2/sff2.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sff2)
install(EXPORT sff2Targets NAMESPACE sff2::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sff2)
configure_package_config_file(cmake/sff2Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/sff2Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sff2)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/sff2ConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/sff2Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/sff2ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sff2)
