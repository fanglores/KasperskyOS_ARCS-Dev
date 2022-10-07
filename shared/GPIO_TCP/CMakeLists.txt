cmake_minimum_required (VERSION 3.12)
project (net_server_gpio)

# Initialize CMake library for Kaspersky OS SDK.
include (platform)
initialize_platform ()

# Toolchain for work with NK-parser.
include (platform/nk)

# Connect artifact installation functions.
include (platform/install)


# Platform definition
include (platform/options)


if ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm|aarch64")
    # This command will find the BSP package components
    # (BSP_HW_ENTITY, BSP_QEMU_ENTITY and BSP_CLIENT_LIB)
    find_package (bsp REQUIRED)
    include_directories (${bsp_INCLUDE})
endif ()

# This command will find GPIO client lib
find_package (gpio REQUIRED COMPONENTS CLIENT_LIB ENTITY)
include_directories (${gpio_INCLUDE})


if (KOS)
    # Attach a package with the virtual file system implementation.
    find_package (vfs REQUIRED)
    include_directories (${vfs_INCLUDE})

    # Attach a package with the DNET file system implementation.
    find_package (dnet REQUIRED COMPONENTS IMPLEMENTATION_LIB)

    add_subdirectory (server)
    add_subdirectory (env)
    add_subdirectory (vfs)
endif ()

if (KOS)
    # Binary image of KasperskyOS.
    add_subdirectory (einit)
endif ()
