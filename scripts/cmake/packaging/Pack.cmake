if(_IS_SUBPROJECT)
    include(CPack)
    return()
endif()

# Put ogs installs into its own component and then only install this component
# (avoids third-party installs from CPM).
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME ogs)
set(CPACK_INSTALL_CMAKE_PROJECTS
    "${PROJECT_BINARY_DIR};${PROJECT_NAME};${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME};/"
)

option(OGS_INSTALL_DEPENDENCIES "Package dependencies." ON)
# Can be tweaked with the CMake variables OGS_INSTALL_DEPENDENCIES_PRE_EXCLUDES
# and OGS_INSTALL_DEPENDENCIES_POST_EXCLUDES. Can contain a semicolon-separated
# list. See PRE_EXCLUDE_REGEXES and POST_EXCLUDE_REGEXES in CMake docs:
# https://cmake.org/cmake/help/latest/command/file.html#get-runtime-dependencies

# Packaging setup
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OGS-6 THM/C Simulator")
set(CPACK_PACKAGE_VENDOR "OpenGeoSys Community (http://www.opengeosys.org)")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "OGS-${OGS_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")
# set(CPACK_RESOURCE_FILE_WELCOME "${PROJECT_SOURCE_DIR}/README.md")

# Package file name
if(OGS_USE_PYTHON)
    list(APPEND SUFFIX_LIST "python-${Python_VERSION}")
endif()
if(OGS_BUILD_GUI)
    list(APPEND SUFFIX_LIST "de")
endif()
if(OGS_BUILD_UTILS)
    list(APPEND SUFFIX_LIST "utils")
endif()
if(OGS_USE_MPI)
    list(APPEND SUFFIX_LIST "mpi")
endif()
string(REPLACE ";" "-" SUFFIX "${SUFFIX_LIST}")

if(APPLE)
    string(REGEX MATCH "(^[0-9]*)" TMP ${CMAKE_SYSTEM_VERSION})
    math(EXPR OSX_VERSION_MINOR "${CMAKE_MATCH_1} - 4")
    set(CPACK_PACKAGE_FILE_NAME
        "ogs-${OGS_VERSION}-OSX-10.${OSX_VERSION_MINOR}-${SUFFIX}"
    )
    set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})
else()
    set(CPACK_PACKAGE_FILE_NAME "ogs-${OGS_VERSION}-${CMAKE_SYSTEM}-${SUFFIX}")
endif()

if(WIN32)
    include(packaging/PackagingWin)
endif()
if(UNIX)
    if(APPLE)
        include(packaging/PackagingMac)
    else()
        include(packaging/PackagingLinux)
    endif()
endif()

include(packaging/PackagingDependencies)

include(CPack)

if(OGS_USE_CONAN)
    # Install Qt platform shared libraries
    install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/platforms
            DESTINATION bin OPTIONAL
    )
endif()

if(OGS_USE_PYTHON)
    if(WIN32)
        file(GLOB PYTHON_RUNTIME_LIBS "${Python_RUNTIME_LIBRARY_DIRS}/*.dll")
        message(STATUS "Install Python into bin-dir: ${PYTHON_RUNTIME_LIBS}")
        install(FILES ${PYTHON_RUNTIME_LIBS} DESTINATION bin)
        file(COPY ${PYTHON_RUNTIME_LIBS}
             DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
    else()
        file(INSTALL ${Python_LIBRARIES} DESTINATION ${CMAKE_INSTALL_LIBDIR}
             FOLLOW_SYMLINK_CHAIN
        )
    endif()
endif()

configure_file(Documentation/README.txt.in ${PROJECT_BINARY_DIR}/README.txt)
install(FILES ${PROJECT_BINARY_DIR}/README.txt DESTINATION .)

install(FILES ${PROJECT_BINARY_DIR}/CMakeCache.txt TYPE INFO)
install(FILES ${PROJECT_BINARY_DIR}/cmake_args TYPE INFO OPTIONAL)
if(NOT WIN32)
    install(
       CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR} --config ${CMAKE_BUILD_TYPE} -t write-licenses)"
    )
endif()
install(FILES ${PROJECT_BINARY_DIR}/third_party_licenses_cpm.txt TYPE INFO)
if(EXISTS ${PROJECT_BINARY_DIR}/third_party_licenses_ext.txt)
    install(FILES ${PROJECT_BINARY_DIR}/third_party_licenses_ext.txt TYPE INFO)
endif()
