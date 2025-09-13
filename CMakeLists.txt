cmake_minimum_required(VERSION 3.16...3.29)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Do not build in-source. Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source.")
endif()

project(
  simplejson
  VERSION 1.1.10
  LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(GNUInstallDirs)
include(CMakeDependentOption)
include(cmake/CPM.cmake)
include(Warnings)

cmake_dependent_option(SIMPLEJSON_BUILD_EXAMPLES
    "Enable ${PROJECT_NAME} project example targets" OFF
    "SIMPLEJSON_BUILD_EXAMPLES" OFF
)

cmake_dependent_option(SIMPLEJSON_BUILD_TESTS
    "Enable ${PROJECT_NAME} project tests targets" OFF
    "SIMPLEJSON_BUILD_TESTS" OFF
)

CPMAddPackage("gh:StableCoder/cmake-scripts#24.04")
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")

CPMAddPackage(
  NAME catch2
  VERSION 3.10.0
  GITHUB_REPOSITORY catchorg/Catch2
)

set(CATCH_BUILD_TESTING OFF)
set(CATCH_INSTALL_DOCS OFF)

list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/contrib")

if(USE_SANITIZER)
    include(${cmake-scripts_SOURCE_DIR}/sanitizers.cmake)
endif()

include(${cmake-scripts_SOURCE_DIR}/tools.cmake)


# Define the header-only library
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(SIMPLEJSON_BUILD_TESTS)
  file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
  file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cc")
  add_executable(Test_Suite "${CMAKE_CURRENT_SOURCE_DIR}/test/test.cc")
  set_target_properties(Test_Suite PROPERTIES CXX_STANDARD 17)

  target_link_libraries(Test_Suite PRIVATE simplejson Catch2WithMain)

  # C++20
  add_executable(Test_Suite_CPP20 "${CMAKE_CURRENT_SOURCE_DIR}/test/test.cc")
  set_target_properties(Test_Suite_CPP20 PROPERTIES CXX_STANDARD 20)
  target_link_libraries(Test_Suite_CPP20 PRIVATE simplejson Catch2WithMain)

endif()

if(SIMPLEJSON_BUILD_EXAMPLES)

  file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
  file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cc")

  foreach(example_source ${sources})
      get_filename_component(example_name ${example_source} NAME_WE)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DDEBUG")
      add_executable(${example_name} ${headers} ${example_source})
      set_target_properties(${example_name} PROPERTIES CXX_STANDARD 17)
      target_set_warnings(${example_name} ENABLE ALL AS_ERROR ALL DISABLE Annoying)

      target_include_directories(
        ${example_name} PUBLIC $<BUILD_INTERFACE:${${example_name}_SOURCE_DIR}>
                              $<INSTALL_INTERFACE:${example_name}-${PROJECT_VERSION}>
      )

      target_link_libraries(${example_name} PRIVATE simplejson)
  endforeach()
endif()
