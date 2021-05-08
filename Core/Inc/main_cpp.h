#ifndef MAIN_CPP_H
#define MAIN_CPP_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Delay_us(uint32_t delay);

void SetTempPinMode(bool input);
bool ReadTempPin();
void WriteTempPin(bool state);
bool WaitForTempPin(bool state, uint32_t timeout_us);
uint32_t WaitForTempPinPulse(bool state);
bool ReadTempData(float* humidity, float* temp);

bool Print(const char* format, ...);
bool PrintLine(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif
