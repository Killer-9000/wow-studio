cmake_minimum_required ( VERSION 3.10 )

# Set the project name
project ( wowstudio )

# Specify the C++ standard
set ( CMAKE_CXX_STANDARD 20 )
set ( CMAKE_CXX_STANDARD_REQUIRED True )
set ( USE_FOLDERS ON)
set ( PREDEFINED_TARGETS_FOLDER "CMakePredefinedTargets")
set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Compiler options
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL")
endif()

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
        Diligent-GraphicsEngineVk-static
        Diligent-TargetPlatform
        Diligent-Common

		EnTT::EnTT
        fmt::fmt
		glm::glm
        tsl::robin_map
        storm
        
        Vulkan::Headers
)

# Copy resource files.
add_custom_command(
    TARGET wowstudio PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -R
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