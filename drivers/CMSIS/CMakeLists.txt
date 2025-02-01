file(GLOB_RECURSE includes *.h)
add_library(DriversCMSIS STATIC ${includes})
target_include_directories(DriversCMSIS PUBLIC Include PUBLIC Device/ST/STM32L4xx/Include)
set_target_properties(DriversCMSIS PROPERTIES LINKER_LANGUAGE C)