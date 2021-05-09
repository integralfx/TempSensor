#ifndef MAIN_CPP_H
#define MAIN_CPP_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t Timer_100ns();
uint32_t Timer_us();
void Delay_100ns(uint32_t multiplier);
void Delay_us(uint32_t delay);

void SetTempPinMode(bool input);
bool ReadTempPin();
void WriteTempPin(bool state);
bool WaitForTempPin(bool state, uint32_t timeout_us);
uint32_t WaitForTempPinPulse(bool state);
bool ReadTempData(float* humidity, float* temp);

void LCD_Clear();
void LCD_ReturnHome();
void LCD_SetControl(bool display_on, bool cursor_on, bool cursor_blink);
void LCD_SetFunction(bool data_8bit, bool two_lines, bool font_10dots);
void LCD_MoveScreen(bool right);
void LCD_SetCursor(int row, int col, bool two_lines);
void LCD_WriteChar(char c);
void LCD_WriteStr(const char* str);

bool Print(const char* format, ...);
bool PrintLine(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif
