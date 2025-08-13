# Define the executable name
set(EXEC_NAME flatten_edep_info)

# Add the executable
add_executable(${EXEC_NAME} flatten_edep_info.cxx)

# Link against ROOT libraries and cenns_io
# We only link to cenns_io to minimize dependencies
target_link_libraries(${EXEC_NAME} 
    PRIVATE
    cenns::cenns_io
    ROOT::Core
    ROOT::Tree
    ROOT::RIO
    ROOT::Hist
)

# Set include directories if needed
# The cenns::cenns_io target should already provide necessary includes
target_include_directories(${EXEC_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Optional: Set output directory for the executable
set_target_properties(${EXEC_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Print build info for this target
message(STATUS "Configured parser: ${EXEC_NAME}")