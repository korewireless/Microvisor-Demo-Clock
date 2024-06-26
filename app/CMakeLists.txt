cmake_minimum_required(VERSION 3.14)

# Set application data
set(APP "Microvisor Clock Demo")
set(VERSION_NUMBER "0.1.4")
set(BUILD_NUMBER "1")

message("Building ${APP} ${VERSION_NUMBER} build ${BUILD_NUMBER}")

# Pass in version data
configure_file(app_version.in app_version.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Compile app source code file(s)
add_executable(${PROJECT_NAME}
    main.cpp
    clock.cpp
    i2c.cpp
    ht16k33.cpp
    config.cpp
    logging.c
    uart_logging.c
    stm32u5xx_hal_timebase_tim_template.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_OPTIONS -Wstack-usage=32768)

# Link built libraries
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    ST_Code
    Microvisor-HAL-STM32U5
    ArduinoJson
)

# Optional informational and additional format generation
add_custom_command(OUTPUT EXTRA_FILES DEPENDS ${PROJECT_NAME}
    COMMAND mv "${PROJECT_NAME}" "${PROJECT_NAME}.elf"
    COMMAND ${CMAKE_SIZE} --format=berkeley "${PROJECT_NAME}.elf"
    COMMAND ${CMAKE_OBJDUMP} -h -S "${PROJECT_NAME}.elf" > "${PROJECT_NAME}.list"
    COMMAND ${CMAKE_OBJCOPY} --output-target ihex "${PROJECT_NAME}.elf" "${PROJECT_NAME}.hex"
    COMMAND ${CMAKE_OBJCOPY} --input-target ihex --output-target binary --gap-fill 0xFF "${PROJECT_NAME}.hex" "${PROJECT_NAME}.bin"
)

# Prepare the additional files
add_custom_target(extras ALL DEPENDS EXTRA_FILES)
