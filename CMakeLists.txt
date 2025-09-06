cmake_minimum_required ( VERSION 3.10 )

# Set the project name
project ( wowstudio )

# Specify the C++ standard
set ( CMAKE_CXX_STANDARD 20 )
set ( CMAKE_CXX_STANDARD_REQUIRED True )
set ( USE_FOLDERS ON)
set ( PREDEFINED_TARGETS_FOLDER "CMakePredefinedTargets")
set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set ( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Compiler options
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL")
endif()

# The sanitizer is really good for picking up random shit which will cause errors down
#   the line, but can mess with things, such as hard crashing with Tracy.
# Enable UBsan
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
# Allow strange alignments
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-sanitize=alignment")
# Make encountering UB an unrecoverable error
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-sanitize-recover=all")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src")

# Platform specific stuff
if (WIN32)
  add_definitions(-DPLATFORM_WIN32)
elseif(UNIX)
  add_definitions(-DPLATFORM_LINUX)
else()
  add_definitions(-DPLATFORM_UNK)
endif()

# find Vulkan SDK
find_package( Vulkan REQUIRED )

# Require Vulkan version >= 1.3.256 (earliest version when the Vulkan module was available)
#if( ${Vulkan_VERSION} VERSION_LESS "1.3.256" )
#  message( FATAL_ERROR "Minimum required Vulkan version for C++ modules is 1.3.256. "
#           "Found ${Vulkan_VERSION}."
#  )
#endif()

file(GLOB_RECURSE sources ${CMAKE_SOURCE_DIR}/src/*)

include_directories (
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/graphics/imgui
)

# Finally add our executable
add_executable ( wowstudio
    ${sources}
)

# Add dependencies
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor)

target_include_directories( wowstudio PUBLIC ${GLM_INCLUDE_DIRS} )
target_link_libraries(wowstudio
  PUBLIC
    EnTT::EnTT
    fmt::fmt
    glm::glm
    nlohmann_json
    tsl::robin_map
    SDL3::SDL3
    storm
    Tracy::TracyClient
    Vulkan::Vulkan
    VulkanMemoryAllocator
    VulkanMemoryAllocator-Hpp
)

# Copy resource files.
add_custom_command(
    TARGET wowstudio PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf
            ${CMAKE_CURRENT_BINARY_DIR}/resources/
)
add_custom_command(
    TARGET wowstudio PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources/
            ${CMAKE_CURRENT_BINARY_DIR}/resources/
)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()