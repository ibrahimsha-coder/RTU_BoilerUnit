###
# © 2025 BoatLoadMinds PVT LMT. All Rights Reserved.
###

set(ROOT_DIR "${CMAKE_SOURCE_DIR}/CoreCode")

file(GLOB_RECURSE EXTRA_SOURCES 
    ${ROOT_DIR}/MCAL/Peripheral/*.c
    ${ROOT_DIR}/Platform/Sensors/*.c
    ${ROOT_DIR}/Platform/Devices/*.c
    ${ROOT_DIR}/Services/InternalServices/*.c
    ${ROOT_DIR}/Services/ExternalServices/*.c
    ${ROOT_DIR}/Application/Processes/*.c
    ${ROOT_DIR}/Common/*.c
)

set(COMPONENT_SRCS "main.c" ${EXTRA_SOURCES})

set(COMPONENT_ADD_INCLUDEDIRS
    "${ROOT_DIR}/MCAL/Peripheral"
    "${ROOT_DIR}/Platform/Sensors"
    "${ROOT_DIR}/Platform/Devices"
    "${ROOT_DIR}/Services/InternalServices/Subsystem"
    "${ROOT_DIR}/Services/ExternalServices"
    "${ROOT_DIR}/Application/Processes"
    "${ROOT_DIR}/Common"
    "."
)

register_component()