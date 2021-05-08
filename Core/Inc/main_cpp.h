#ifndef MAIN_CPP_H
#define MAIN_CPP_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void SetTempPinMode(bool input);
void Delay_us(uint32_t delay);
bool WaitForTempPin(bool state, uint32_t timeout_us);
void ReadTempData();
void WriteTempPin(bool state);
bool ReadTempPin();

bool Print(const char* format, ...);
bool PrintLine(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif
