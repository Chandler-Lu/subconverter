cmake_minimum_required(VERSION 3.15)
project(subconverter_conversion_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
get_filename_component(SUBCONVERTER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
list(APPEND CMAKE_MODULE_PATH "${SUBCONVERTER_SOURCE_DIR}/cmake")
find_package(yaml-cpp CONFIG REQUIRED)
find_package(toml11 CONFIG REQUIRED)
find_package(Rapidjson REQUIRED)
find_package(PCRE2 REQUIRED)
find_package(Threads REQUIRED)
if(TARGET yaml-cpp::yaml-cpp)
    set(YAML_CPP_TEST_TARGET yaml-cpp::yaml-cpp)
else()
    set(YAML_CPP_TEST_TARGET yaml-cpp)
endif()

# Use the production parser and exporters without the HTTP server or JS runtime.
add_executable(surge_vmess_test
    surge_vmess_test.cpp
    ../src/generator/config/ruleconvert.cpp
    ../src/generator/config/subexport.cpp
    ../src/generator/template/templates.cpp
    ../src/lib/wrapper.cpp
    ../src/parser/subparser.cpp
    ../src/utils/base64/base64.cpp
    ../src/utils/codepage.cpp
    ../src/utils/logger.cpp
    ../src/utils/md5/md5.cpp
    ../src/utils/network.cpp
    ../src/utils/regexp.cpp
    ../src/utils/string.cpp
    ../src/utils/urlencode.cpp)
target_include_directories(surge_vmess_test PRIVATE
    "${SUBCONVERTER_SOURCE_DIR}/src" "${SUBCONVERTER_SOURCE_DIR}/include"
    ${RAPIDJSON_INCLUDE_DIRS} ${PCRE2_INCLUDE_DIRS})
target_compile_definitions(surge_vmess_test PRIVATE NO_JS_RUNTIME NO_WEBGET HAVE_TO_STRING)
target_link_libraries(surge_vmess_test PRIVATE
    ${YAML_CPP_TEST_TARGET} toml11::toml11 ${PCRE2_LIBRARIES} Threads::Threads)
if(WIN32)
    target_link_libraries(surge_vmess_test PRIVATE ws2_32)
endif()

enable_testing()
add_test(NAME surge_vmess COMMAND surge_vmess_test)
