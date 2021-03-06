cmake_minimum_required(VERSION 2.8)

project(Catch)

# define some folders
get_filename_component(CATCH_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PATH)
get_filename_component(CATCH_DIR "${CATCH_DIR}" PATH)
set(SELF_TEST_DIR ${CATCH_DIR}/projects/SelfTest)
if(USE_CPP11)
    ## We can't turn this on by default, since it breaks on travis
    message(STATUS "Enabling C++11")
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

# define the sources of the self test
set(SOURCES
        ${SELF_TEST_DIR}/ApproxTests.cpp
        ${SELF_TEST_DIR}/BDDTests.cpp
        ${SELF_TEST_DIR}/ClassTests.cpp
        ${SELF_TEST_DIR}/ConditionTests.cpp
        ${SELF_TEST_DIR}/ExceptionTests.cpp
        ${SELF_TEST_DIR}/GeneratorTests.cpp
        ${SELF_TEST_DIR}/MessageTests.cpp
        ${SELF_TEST_DIR}/MiscTests.cpp
        ${SELF_TEST_DIR}/PartTrackerTests.cpp
        ${SELF_TEST_DIR}/TestMain.cpp
        ${SELF_TEST_DIR}/TrickyTests.cpp
        ${SELF_TEST_DIR}/VariadicMacrosTests.cpp
        ${SELF_TEST_DIR}/EnumToString.cpp
        ${SELF_TEST_DIR}/ToStringPair.cpp
        ${SELF_TEST_DIR}/ToStringVector.cpp
        ${SELF_TEST_DIR}/ToStringWhich.cpp
        ${SELF_TEST_DIR}/ToStringTuple.cpp
        ${SELF_TEST_DIR}/CmdLineTests.cpp
        ${SELF_TEST_DIR}/TagAliasTests.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_common.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_console_colour.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_debugger.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_interfaces_capture.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_interfaces_config.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_interfaces_exception.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_interfaces_generators.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_interfaces_registry_hub.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_interfaces_reporter.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_interfaces_runner.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_interfaces_testcase.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_message.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_option.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_ptr.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_stream.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_streambuf.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_test_spec.cpp
        ${SELF_TEST_DIR}/SurrogateCpps/catch_xmlwriter.cpp
        )

# configure the executable
include_directories(${CATCH_DIR}/include)
add_executable(SelfTest ${SOURCES})

# configure unit tests via CTest
enable_testing()
add_test(NAME RunTests COMMAND SelfTest)

add_test(NAME ListTests COMMAND SelfTest --list-tests)
set_tests_properties(ListTests PROPERTIES PASS_REGULAR_EXPRESSION "[0-9]+ test cases")

add_test(NAME ListTags COMMAND SelfTest --list-tags)
set_tests_properties(ListTags PROPERTIES PASS_REGULAR_EXPRESSION "[0-9]+ tags")
