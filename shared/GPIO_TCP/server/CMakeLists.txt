project (client)

# Toolchain for work with NK-parser.
include (platform/nk)

# Set compilation flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

nk_build_edl_files (server_edl_files NK_MODULE "server" EDL "${CMAKE_SOURCE_DIR}/resources/edl/Server.edl")

include(CMakePrintHelpers)
cmake_print_variables(server_edl_files)


add_executable (Server "src/server.cpp")
add_dependencies (Server server_edl_files)
target_link_libraries (Server ${${PROJECT_NAME}_LIBS} ${vfs_CLIENT_LIB})
set_target_properties (Server PROPERTIES ${vfs_ENTITY}_REPLACEMENT "")



target_link_libraries (Server ${gpio_CLIENT_LIB})

if ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm|aarch64")
    target_link_libraries (Server ${bsp_CLIENT_LIB})
endif ()
