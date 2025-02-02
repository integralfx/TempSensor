#pragma once
#include "LCD.hpp"

class LCD_TC1602A : public ILCDBase<LCD_TC1602A>
{
public:
    bool SendCommand(const LCDCommand& cmd);
};