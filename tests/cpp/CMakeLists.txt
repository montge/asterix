# C++ Unit Tests with Google Test
# DO-278A AL-3 Compliance Testing

cmake_minimum_required(VERSION 3.14)

# Download and configure Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src/asterix
    ${CMAKE_SOURCE_DIR}/src/engine
)

# Test executables
add_executable(test_category
    test_category.cpp
)

add_executable(test_dataitem
    test_dataitem.cpp
)

add_executable(test_utils
    test_utils.cpp
)

add_executable(test_dataitembits
    test_dataitembits.cpp
)

add_executable(test_dataitemformatexplicit
    test_dataitemformatexplicit.cpp
)

add_executable(test_datarecord
    test_datarecord.cpp
)

add_executable(test_dataitemformatrepetitive
    test_dataitemformatrepetitive.cpp
)

add_executable(test_dataitemformatvariable
    test_dataitemformatvariable.cpp
)

add_executable(test_dataitemformatbds
    test_dataitemformatbds.cpp
)

add_executable(test_dataitemformatcompound
    test_dataitemformatcompound.cpp
)

add_executable(test_asterixdefinition
    test_asterixdefinition.cpp
)

add_executable(test_tracer
    test_tracer.cpp
)

add_executable(test_datablock
    test_datablock.cpp
)

add_executable(test_dataitemformatfixed
    test_dataitemformatfixed.cpp
)

add_executable(test_xmlparser
    test_xmlparser.cpp
)

add_executable(test_inputparser
    test_inputparser.cpp
)

add_executable(test_asterixdata
    test_asterixdata.cpp
)

add_executable(test_dataitemdescription
    test_dataitemdescription.cpp
)

add_executable(test_uap
    test_uap.cpp
)

add_executable(test_uapitem
    test_uapitem.cpp
)

# Integration tests
add_executable(test_integration_cat048
    test_integration_cat048.cpp
)

add_executable(test_integration_cat062
    test_integration_cat062.cpp
)

add_executable(test_integration_cat065
    test_integration_cat065.cpp
)

add_executable(test_integration_error_handling
    test_integration_error_handling.cpp
)

add_executable(test_integration_pcap_format
    test_integration_pcap_format.cpp
)

add_executable(test_integration_cat011
    test_integration_cat011.cpp
)

add_executable(test_integration_cat015
    test_integration_cat015.cpp
)

add_executable(test_integration_cat019
    test_integration_cat019.cpp
)

add_executable(test_integration_cat025
    test_integration_cat025.cpp
)

add_executable(test_integration_cat031
    test_integration_cat031.cpp
)

add_executable(test_integration_cat032
    test_integration_cat032.cpp
)

add_executable(test_integration_cat063
    test_integration_cat063.cpp
)

add_executable(test_integration_cat205
    test_integration_cat205.cpp
)

add_executable(test_integration_cat240
    test_integration_cat240.cpp
)

add_executable(test_integration_cat247
    test_integration_cat247.cpp
)

add_executable(test_integration_cat252
    test_integration_cat252.cpp
)

# List of all test executables for bulk operations
set(ALL_TEST_TARGETS
    test_category
    test_dataitem
    test_utils
    test_dataitembits
    test_dataitemformatexplicit
    test_datarecord
    test_dataitemformatrepetitive
    test_dataitemformatvariable
    test_dataitemformatbds
    test_dataitemformatcompound
    test_asterixdefinition
    test_tracer
    test_datablock
    test_dataitemformatfixed
    test_xmlparser
    test_inputparser
    test_asterixdata
    test_dataitemdescription
    test_uap
    test_uapitem
    test_integration_cat048
    test_integration_cat062
    test_integration_cat065
    test_integration_error_handling
    test_integration_pcap_format
    test_integration_cat011
    test_integration_cat015
    test_integration_cat019
    test_integration_cat025
    test_integration_cat031
    test_integration_cat032
    test_integration_cat063
    test_integration_cat205
    test_integration_cat240
    test_integration_cat247
    test_integration_cat252
)

# Add WIRESHARK_WRAPPER compile definition to all tests
# This is required for ABI compatibility with the asterix library which is
# compiled with WIRESHARK_WRAPPER defined, adding extra members to DataItemFormat
foreach(test_target ${ALL_TEST_TARGETS})
    target_compile_definitions(${test_target} PRIVATE WIRESHARK_WRAPPER)
endforeach()

# Link tests with Google Test and asterix library
if(TARGET asterix_static)
    target_link_libraries(test_category GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_dataitem GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_utils GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_dataitembits GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_dataitemformatexplicit GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_datarecord GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_dataitemformatrepetitive GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_dataitemformatvariable GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_dataitemformatbds GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_dataitemformatcompound GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_asterixdefinition GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_tracer GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_datablock GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_dataitemformatfixed GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_xmlparser GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_inputparser GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_asterixdata GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_dataitemdescription GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_uap GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_uapitem GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat048 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat062 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat065 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_error_handling GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_pcap_format GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat011 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat015 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat019 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat025 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat031 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat032 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat063 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat205 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat240 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat247 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat252 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
else()
    message(WARNING "asterix_static target not found, C++ tests will not be built")
endif()

# Add tests to CTest
# Note: WORKING_DIRECTORY is set to the build directory so tests can find
# ../asterix/config/ relative paths correctly
include(GoogleTest)
gtest_discover_tests(test_category WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_dataitem WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_utils WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_dataitembits WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_dataitemformatexplicit WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_datarecord WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_dataitemformatrepetitive WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_dataitemformatvariable WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_dataitemformatbds WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_dataitemformatcompound WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_asterixdefinition WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_tracer WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_datablock WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_dataitemformatfixed WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_xmlparser WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_inputparser WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_asterixdata WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_dataitemdescription WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_uap WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_uapitem WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat048 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat062 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat065 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_error_handling WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_pcap_format WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat011 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat015 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat019 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat025 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat031 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat032 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat063 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat205 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat240 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat247 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
gtest_discover_tests(test_integration_cat252 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# Coverage flags (if enabled)
if(ENABLE_COVERAGE)
    target_compile_options(test_category PRIVATE --coverage)
    target_compile_options(test_dataitem PRIVATE --coverage)
    target_compile_options(test_utils PRIVATE --coverage)
    target_compile_options(test_dataitembits PRIVATE --coverage)
    target_compile_options(test_dataitemformatexplicit PRIVATE --coverage)
    target_compile_options(test_datarecord PRIVATE --coverage)
    target_compile_options(test_dataitemformatrepetitive PRIVATE --coverage)
    target_compile_options(test_dataitemformatvariable PRIVATE --coverage)
    target_compile_options(test_dataitemformatbds PRIVATE --coverage)
    target_compile_options(test_dataitemformatcompound PRIVATE --coverage)
    target_compile_options(test_asterixdefinition PRIVATE --coverage)
    target_compile_options(test_tracer PRIVATE --coverage)
    target_compile_options(test_datablock PRIVATE --coverage)
    target_compile_options(test_dataitemformatfixed PRIVATE --coverage)
    target_compile_options(test_xmlparser PRIVATE --coverage)
    target_compile_options(test_inputparser PRIVATE --coverage)
    target_compile_options(test_asterixdata PRIVATE --coverage)
    target_compile_options(test_dataitemdescription PRIVATE --coverage)
    target_compile_options(test_uap PRIVATE --coverage)
    target_compile_options(test_uapitem PRIVATE --coverage)
    target_compile_options(test_integration_cat048 PRIVATE --coverage)
    target_compile_options(test_integration_cat062 PRIVATE --coverage)
    target_compile_options(test_integration_cat065 PRIVATE --coverage)
    target_compile_options(test_integration_error_handling PRIVATE --coverage)
    target_compile_options(test_integration_pcap_format PRIVATE --coverage)
    target_compile_options(test_integration_cat011 PRIVATE --coverage)
    target_compile_options(test_integration_cat015 PRIVATE --coverage)
    target_compile_options(test_integration_cat019 PRIVATE --coverage)
    target_compile_options(test_integration_cat025 PRIVATE --coverage)
    target_compile_options(test_integration_cat031 PRIVATE --coverage)
    target_compile_options(test_integration_cat032 PRIVATE --coverage)
    target_compile_options(test_integration_cat063 PRIVATE --coverage)
    target_compile_options(test_integration_cat205 PRIVATE --coverage)
    target_compile_options(test_integration_cat240 PRIVATE --coverage)
    target_compile_options(test_integration_cat247 PRIVATE --coverage)
    target_compile_options(test_integration_cat252 PRIVATE --coverage)

    target_link_options(test_category PRIVATE --coverage)
    target_link_options(test_dataitem PRIVATE --coverage)
    target_link_options(test_utils PRIVATE --coverage)
    target_link_options(test_dataitembits PRIVATE --coverage)
    target_link_options(test_dataitemformatexplicit PRIVATE --coverage)
    target_link_options(test_datarecord PRIVATE --coverage)
    target_link_options(test_dataitemformatrepetitive PRIVATE --coverage)
    target_link_options(test_dataitemformatvariable PRIVATE --coverage)
    target_link_options(test_dataitemformatbds PRIVATE --coverage)
    target_link_options(test_dataitemformatcompound PRIVATE --coverage)
    target_link_options(test_asterixdefinition PRIVATE --coverage)
    target_link_options(test_tracer PRIVATE --coverage)
    target_link_options(test_datablock PRIVATE --coverage)
    target_link_options(test_dataitemformatfixed PRIVATE --coverage)
    target_link_options(test_xmlparser PRIVATE --coverage)
    target_link_options(test_inputparser PRIVATE --coverage)
    target_link_options(test_asterixdata PRIVATE --coverage)
    target_link_options(test_dataitemdescription PRIVATE --coverage)
    target_link_options(test_uap PRIVATE --coverage)
    target_link_options(test_uapitem PRIVATE --coverage)
    target_link_options(test_integration_cat048 PRIVATE --coverage)
    target_link_options(test_integration_cat062 PRIVATE --coverage)
    target_link_options(test_integration_cat065 PRIVATE --coverage)
    target_link_options(test_integration_error_handling PRIVATE --coverage)
    target_link_options(test_integration_pcap_format PRIVATE --coverage)
    target_link_options(test_integration_cat011 PRIVATE --coverage)
    target_link_options(test_integration_cat015 PRIVATE --coverage)
    target_link_options(test_integration_cat019 PRIVATE --coverage)
    target_link_options(test_integration_cat025 PRIVATE --coverage)
    target_link_options(test_integration_cat031 PRIVATE --coverage)
    target_link_options(test_integration_cat032 PRIVATE --coverage)
    target_link_options(test_integration_cat063 PRIVATE --coverage)
    target_link_options(test_integration_cat205 PRIVATE --coverage)
    target_link_options(test_integration_cat240 PRIVATE --coverage)
    target_link_options(test_integration_cat247 PRIVATE --coverage)
    target_link_options(test_integration_cat252 PRIVATE --coverage)
endif()
