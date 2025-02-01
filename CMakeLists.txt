cmake_minimum_required(VERSION 3.31)
project(TempSensor VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_language(ASM)

set(LINKER_SCRIPT            "../STM32L452REIX_FLASH.ld")
set(MCPU                     "-mcpu=Cortex-M4")
set(MFPU                     "-mfpu=fpv4-sp-d16")
set(MFLOAT_ABI               "")
set(RUNTIME_LIBRARY          "--specs=nano.specs")
set(RUNTIME_LIBRARY_SYSCALLS "--specs=nosys.specs")

set(CMAKE_EXECUTABLE_SUFFIX ".elf")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
add_compile_options(${MCPU} ${MFPU} ${MFLOAT_ABI} ${RUNTIME_LIBRARY} -mthumb -Wall -Werror)
add_link_options(-T${LINKER_SCRIPT} ${RUNTIME_LIBRARY_SYSCALLS} -Wl,-Map=test.map -Wl,--gc-sections -static -Wl,--start-group -lc -lm -Wl,--end-group -u _printf_float)
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp")

file(GLOB_RECURSE sources src/*.c src/*.cpp)
add_executable(TempSensor ${sources} Startup/startup_stm32l452reix.s)

target_include_directories(TempSensor PUBLIC inc)
target_link_libraries(TempSensor PUBLIC Drivers)
target_compile_definitions(TempSensor PUBLIC STM32L452xx)

add_subdirectory(config)
add_subdirectory(Drivers)