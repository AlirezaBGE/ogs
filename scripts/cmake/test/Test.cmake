# Find tools and data
find_program(DIFF_TOOL_PATH diff)
find_program(TIME_TOOL_PATH time)
find_program(GREP_TOOL_PATH grep)
find_program(BASH_TOOL_PATH bash)
find_program(MPIRUN_TOOL_PATH mpirun)

if(NOT TIME_TOOL_PATH)
    message(
        STATUS
            "time-command is required for time wrapper but was not found! All corresponding tests are disabled."
    )
endif()
if(NOT MPIRUN_TOOL_PATH)
    message(
        STATUS
            "mpirun is required for mpirun wrapper but was not found! All corresponding tests are disabled."
    )
endif()
if(NOT DIFF_TOOL_PATH)
    message(
        STATUS
            "diff-command is required for diff tester but was not found! All corresponding tests are disabled."
    )
endif()
if(NOT GREP_TOOL_PATH)
    message(
        STATUS
            "grep-command is required for memcheck tester but was not found! All corresponding tests are disabled."
    )
endif()

enable_testing() # Enable CTest

include(${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake/test/AddTest.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake/test/OgsTest.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake/test/NotebookTest.cmake)

# Check notebook testrunner
NotebookTest(
    NOTEBOOKFILE
    Notebooks/FailingNotebook.ci-skip.ipynb
    RUNTIME
    2
    PROPERTIES
    WILL_FAIL
    TRUE
)

set(_ctest_parameter -T Test --force-new-ctest-process --output-on-failure)
if(CMAKE_CONFIGURATION_TYPES)
    list(APPEND _ctest_parameter --build-config "$<CONFIGURATION>")
endif()

add_custom_target(ctest-cleanup ${CMAKE_COMMAND} -E remove -f Tests/ctest.log)

add_custom_target(
    ctest
    COMMAND ${CMAKE_CTEST_COMMAND} ${_ctest_parameter} --output-log
            Tests/ctest.log -LE large
    DEPENDS ${test_dependencies} ctest-cleanup
    USES_TERMINAL
)

add_custom_target(
    ctest-large-cleanup ${CMAKE_COMMAND} -E remove -f Tests/ctest-large.log
)

add_custom_target(
    ctest-large
    COMMAND ${CMAKE_CTEST_COMMAND} ${_ctest_parameter} --output-log
            Tests/ctest-large.log -L large
    DEPENDS ${test_dependencies} ctest-large-cleanup
    USES_TERMINAL
)

set_directory_properties(
    PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/Tests/Data
)

set_target_properties(
    ctest ctest-large ctest-cleanup ctest-large-cleanup PROPERTIES FOLDER
                                                                   Testing
)

configure_file(
    ${PROJECT_SOURCE_DIR}/scripts/test/buildinfo.in.yaml
    ${PROJECT_BINARY_DIR}/buildinfo.yaml
)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/logs)
