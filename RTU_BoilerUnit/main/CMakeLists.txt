### © 2025 BoatLoadMinds PVT LMT. All Rights Reserved.

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

idf_component_register(
    SRCS "main.c" ${EXTRA_SOURCES}
    INCLUDE_DIRS
        "${ROOT_DIR}/MCAL/Peripheral"
        "${ROOT_DIR}/Platform/Sensors"
        "${ROOT_DIR}/Platform/Devices"
        "${ROOT_DIR}/Services/InternalServices/Subsystem"
        "${ROOT_DIR}/Services/ExternalServices"
        "${ROOT_DIR}/Application/Processes"
        "${ROOT_DIR}/Common"
        "."
)

# ==============================
# 🔥 AWS CERTIFICATE EMBEDDING
# ==============================
target_add_binary_data(${COMPONENT_LIB} "certs/root_cert_auth.crt" TEXT)
target_add_binary_data(${COMPONENT_LIB} "certs/client.crt" TEXT)
target_add_binary_data(${COMPONENT_LIB} "certs/client.key" TEXT)