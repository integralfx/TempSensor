#pragma once

void LCD_Clear();
void LCD_ReturnHome();
void LCD_SetControl(bool display_on, bool cursor_on, bool cursor_blink);
void LCD_SetFunction(bool data_8bit, bool two_lines, bool font_10dots);
void LCD_MoveScreen(bool right);
void LCD_SetCursor(int row, int col, bool two_lines);
void LCD_WriteChar(char c);
void LCD_WriteStr(const char* str);