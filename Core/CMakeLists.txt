file(GLOB_RECURSE sources Src/*.c Src/*.cpp)
add_executable(TempSensor ${sources} Startup/startup_stm32l452reix.s)

target_include_directories(TempSensor PUBLIC Inc)
target_link_libraries(TempSensor PUBLIC Drivers)
target_compile_definitions(TempSensor PUBLIC STM32L452xx)