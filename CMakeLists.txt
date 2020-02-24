/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(thrift-http-ws-server VERSION 1.0.0)
set(COMPANY "BioDataAnalysis GmbH")
set(COPYRIGHT "Copyright (c) BioDataAnalysis GmbH. All rights reserved.")

option(ENABLE_TEST "Build tests" ON)
option(ENABLE_THRIFT_NODEJS "Build thrift nodejs browser client" ON)

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Boost 1.70.0 COMPONENTS program_options system REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(Thrift 0.14.0)
if(THRIFT_FOUND)
    set(THRIFT_COMPILER "${THRIFT_BIN_DIR}/thrift")
else()
    set(WITH_JAVASCRIPT ON)
    set(WITH_NODEJS ON)
    set(BUILD_CPP ON)
    set(WITH_ZLIB ON)
    set(BUILD_TESTING OFF)
    set(BUILD_TUTORIALS OFF)
    set(WITH_JAVA OFF)
    set(WITH_QT5 OFF)
    set(BUILD_AS3 OFF)
    set(BUILD_C_GLIB OFF)
    set(BUILD_PYTHON OFF)
    set(BUILD_HASKELL OFF)
    add_subdirectory(thrift)
    set(THRIFT_COMPILER "thrift-compiler")
endif()

set(SOURCES
    include/bda/ThriftHelper.hh
    src/ThriftHelper.cc
    include/bda/ThriftHTTPWSServer.hh
    src/ThriftHTTPWSServer.cc)

add_library(${PROJECT_NAME} ${SOURCES})
add_library(BDA::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG;_DEBUG;_DEBUG_>
        $<$<PLATFORM_ID:Windows>:NOMINMAX;_CRT_SECURE_NO_DEPRECATE>)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-pedantic;-Wall;-Wextra;-Werror=return-type;-Werror=uninitialized;-Werror=unknown-pragmas>
        $<$<CXX_COMPILER_ID:MSVC>:/W4;/wd4251;/wd4244;/wd4267;/bigobj>)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS NO)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${THRIFT_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        thrift::thrift Threads::Threads)

if(ENABLE_TEST)
    list(APPEND TESTS
        ThriftHTTPWSServerDemo)

    find_package(GTest 1.8.0 REQUIRED)
    enable_testing()

    set(THRIFT_IDL_FILE
        "${CMAKE_CURRENT_SOURCE_DIR}/test/thrift/TestThriftAPI.thrift")

    # automatically generate Thrift c++ sources:
    set(THRIFT_GENCPP_HEADER_FILES_LIST
        "${CMAKE_CURRENT_SOURCE_DIR}/test/gen-cpp/TestThriftAPI.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/test/gen-cpp/TestThriftAPI_constants.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/test/gen-cpp/TestThriftAPI_types.h")
    set(THRIFT_GENCPP_SOURCE_FILES_LIST
        "${CMAKE_CURRENT_SOURCE_DIR}/test/gen-cpp/TestThriftAPI.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/test/gen-cpp/TestThriftAPI_constants.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/test/gen-cpp/TestThriftAPI_types.cpp")
    add_custom_command(
        OUTPUT ${THRIFT_GENCPP_SOURCE_FILES_LIST} ${THRIFT_GENCPP_HEADER_FILES_LIST}
        DEPENDS "${THRIFT_IDL_FILE}"
        COMMAND "${THRIFT_COMPILER}" -strict -recurse -o "${CMAKE_CURRENT_SOURCE_DIR}/test/" --gen cpp:no_skeleton "${THRIFT_IDL_FILE}"
        COMMENT "Generating thrift C++ bindings from ${THRIFT_IDL_FILE}")


    set(ThriftHTTPWSServerDemo_SOURCES
        test/src/ThriftHTTPWSServerDemo.cc
        test/src/TestThriftAPIHandler.cc
        test/src/TestThriftAPIHandler.hh
        test/src/TestThriftAPICloneFactory.cc
        test/src/TestThriftAPICloneFactory.hh
        ${THRIFT_GENCPP_SOURCE_FILES_LIST}
        ${THRIFT_GENCPP_HEADER_FILES_LIST})

    foreach(TESTNAME ${TESTS})
        add_executable(${TESTNAME} ${${TESTNAME}_SOURCES})

        target_include_directories(${TESTNAME}
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_CURRENT_SOURCE_DIR}/test/src
                ${CMAKE_CURRENT_SOURCE_DIR}/test/gen-cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                ${CMAKE_CURRENT_BINARY_DIR})

        target_link_libraries(${TESTNAME}
	        PRIVATE
	            ${PROJECT_NAME} Boost::program_options)

        add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
        set_tests_properties(${TESTNAME} PROPERTIES TIMEOUT 300)
    endforeach()
endif()

if(ENABLE_THRIFT_NODEJS)
    # automatically generate Thrift nodejs sources:
    set(THRIFT_GENJS_HEADER_FILES_LIST
        "${CMAKE_CURRENT_SOURCE_DIR}/browser-nodejs/src/gen-nodejs/TestThriftAPI.js"
        "${CMAKE_CURRENT_SOURCE_DIR}/browser-nodejs/src/gen-nodejs/TestThriftAPI_types.js")
    add_custom_command(
        OUTPUT ${THRIFT_GENJS_HEADER_FILES_LIST}
        DEPENDS "${THRIFT_IDL_FILE}"
        COMMAND "${THRIFT_COMPILER}" -strict -recurse -o "${CMAKE_CURRENT_SOURCE_DIR}/browser-nodejs/src/" --gen js:node "${THRIFT_IDL_FILE}"
        COMMENT "Generating thrift C++ bindings from ${THRIFT_IDL_FILE}")

    add_custom_target(ThriftBrowserClientDemo ALL
        COMMAND npx webpack
        DEPENDS ${THRIFT_GENJS_HEADER_FILES_LIST}
        COMMENT "Creating webpack for browser nodejs thrift"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/browser-nodejs")
endif()

# Export the package for use from the build tree
export(TARGETS ${PROJECT_NAME} NAMESPACE BDA:: FILE cmake/${PROJECT_NAME}Targets.cmake)
export(PACKAGE ${PROJECT_NAME})

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    cmake/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
    cmake/${PROJECT_NAME}Config.cmake.in
    cmake/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION lib/cmake/)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME bda/thrift-http-ws-server_export.h)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)
install(DIRECTORY include/bda DESTINATION include)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/bda/thrift-http-ws-server_export.h DESTINATION include/bda/)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION lib/cmake/)

install(EXPORT ${PROJECT_NAME}Targets
    NAMESPACE BDA:: DESTINATION lib/cmake/)
