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
    target_link_libraries(test_integration_cat048 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat062 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_cat065 GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_error_handling GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
    target_link_libraries(test_integration_pcap_format GTest::gtest_main asterix_static ${EXPAT_LIBRARIES})
else()
    message(WARNING "asterix_static target not found, C++ tests will not be built")
endif()

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(test_category)
gtest_discover_tests(test_dataitem)
gtest_discover_tests(test_utils)
gtest_discover_tests(test_dataitembits)
gtest_discover_tests(test_dataitemformatexplicit)
gtest_discover_tests(test_datarecord)
gtest_discover_tests(test_dataitemformatrepetitive)
gtest_discover_tests(test_dataitemformatvariable)
gtest_discover_tests(test_dataitemformatbds)
gtest_discover_tests(test_dataitemformatcompound)
gtest_discover_tests(test_asterixdefinition)
gtest_discover_tests(test_tracer)
gtest_discover_tests(test_datablock)
gtest_discover_tests(test_integration_cat048)
gtest_discover_tests(test_integration_cat062)
gtest_discover_tests(test_integration_cat065)
gtest_discover_tests(test_integration_error_handling)
gtest_discover_tests(test_integration_pcap_format)

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
    target_compile_options(test_integration_cat048 PRIVATE --coverage)
    target_compile_options(test_integration_cat062 PRIVATE --coverage)
    target_compile_options(test_integration_cat065 PRIVATE --coverage)
    target_compile_options(test_integration_error_handling PRIVATE --coverage)
    target_compile_options(test_integration_pcap_format PRIVATE --coverage)

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
    target_link_options(test_integration_cat048 PRIVATE --coverage)
    target_link_options(test_integration_cat062 PRIVATE --coverage)
    target_link_options(test_integration_cat065 PRIVATE --coverage)
    target_link_options(test_integration_error_handling PRIVATE --coverage)
    target_link_options(test_integration_pcap_format PRIVATE --coverage)
endif()
