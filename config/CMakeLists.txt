add_library(config STATIC inc/stm32l4xx_hal_conf.h)
set_target_properties(config PROPERTIES LINKER_LANGUAGE C)
target_include_directories(config PUBLIC inc)