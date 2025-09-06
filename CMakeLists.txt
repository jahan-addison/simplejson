cmake_minimum_required(VERSION 3.16...3.29)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Do not build in-source. Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source.")
endif()

project(
  simplejson
  VERSION 1.1.0
  LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake") # Make our cmake scripts available

include(GNUInstallDirs)
include(CMakeDependentOption)
include(cmake/CPM.cmake)
include(Warnings)

cmake_dependent_option(BUILD_EXAMPLES
    "Enable ${PROJECT_NAME} project tests targets" OFF
    "BUILD_EXAMPLES" OFF
)
CPMAddPackage("gh:StableCoder/cmake-scripts#24.04")
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")

# Enable sanitizer with `-DUSE_SANITIZER=<VALUE>` argument to CMake
# enables sanitizers support using the the `USE_SANITIZER` flag available values are: Address,
# Memory, MemoryWithOrigins, Undefined, Thread, Leak, 'Address;Undefined'
if(USE_SANITIZER)
    include(${cmake-scripts_SOURCE_DIR}/sanitizers.cmake)
endif()

include(${cmake-scripts_SOURCE_DIR}/tools.cmake)

# Define the header-only library
add_library(${PROJECT_NAME} INTERFACE)

# Specify the public header(s) for the interface library
target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(BUILD_EXAMPLES)

  file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
  file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp")

    foreach(example_source ${sources})
        get_filename_component(example_name ${example_source} NAME_WE)
        add_executable(${example_name} ${example_source} ${headers})
        set_target_properties(${example_name} PROPERTIES CXX_STANDARD 20)

        target_set_warnings(${example_name} ENABLE ALL AS_ERROR ALL DISABLE Annoying)

        target_include_directories(
          ${example_name} PUBLIC $<BUILD_INTERFACE:${${example_name}_SOURCE_DIR}>
                                $<INSTALL_INTERFACE:${example_name}-${PROJECT_VERSION}>
        )

        target_link_libraries(${example_name} PUBLIC simplejson)
    endforeach()
endif()
