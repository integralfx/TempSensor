#include "LCD_TC1602A.hpp"
#include "Pins.hpp"
#include "Time.hpp"
#include "Serial.hpp"
#include "stm32l4xx_hal.h"
#include <array>
#include <span>

namespace
{
    std::array<std::pair<GPIO_TypeDef*, uint16_t>, 8> lcd_data_pins
    { {
        { LCD_D0_GPIO_Port, LCD_D0_Pin },
        { LCD_D1_GPIO_Port, LCD_D1_Pin },
        { LCD_D2_GPIO_Port, LCD_D2_Pin },
        { LCD_D3_GPIO_Port, LCD_D3_Pin },
        { LCD_D4_GPIO_Port, LCD_D4_Pin },
        { LCD_D5_GPIO_Port, LCD_D5_Pin },
        { LCD_D6_GPIO_Port, LCD_D6_Pin },
        { LCD_D7_GPIO_Port, LCD_D7_Pin }
    } };
}

class AutoEnable
{
public:
    [[nodiscard]] AutoEnable(LCD_TC1602A& lcd) noexcept : m_lcd{ lcd }
    {
        m_lcd.SetEnable(true);
    }

    ~AutoEnable() noexcept
    {
        m_lcd.SetEnable(false);
    }

private:
    LCD_TC1602A& m_lcd;
};

void LCD_TC1602A::Init(const LCDInit& init) noexcept
{
    data_t data;
    data.set(5);
    data.set(4, static_cast<bool>(init.data_size));
    data.set(3, static_cast<bool>(init.row_count));
    data.set(2, static_cast<bool>(init.font_type));
    SendWriteCommand(RegisterSelect::Instruction, data);
}

void LCD_TC1602A::SetSettings(const LCDSettings& settings) noexcept
{
    data_t data;
    data.set(3);
    data.set(2, settings.display_on);
    data.set(1, settings.cursor_on);
    data.set(0, settings.cursor_blink);
    SendWriteCommand(RegisterSelect::Instruction, data);
}

void LCD_TC1602A::Clear() noexcept
{
    data_t data;
    data.set(0);
    SendWriteCommand(RegisterSelect::Instruction, data);
}

void LCD_TC1602A::SetAddress(uint8_t address) noexcept
{
    SendWriteCommand(RegisterSelect::Instruction, address);
}

void LCD_TC1602A::Read(uint8_t& out_data) noexcept
{
    auto data = SendReadCommand(RegisterSelect::Data);
    out_data = static_cast<uint8_t>(data.to_ulong());
}

void LCD_TC1602A::Write(uint8_t data) noexcept
{
    SendWriteCommand(RegisterSelect::Data, data);
}

size_t LCD_TC1602A::WriteRow(const std::span<uint8_t>& data) noexcept
{
    SendWriteCommand(RegisterSelect::Data, data);
    return data.size();
}

bool LCD_TC1602A::IsBusy(uint8_t& address_counter) noexcept
{
    static constexpr data_t address_mask{ 0b1111111 };

    auto data = SendReadCommand(RegisterSelect::Instruction);
    address_counter = static_cast<uint8_t>((data & address_mask).to_ulong());
    return data.test(7);
}

bool LCD_TC1602A::WaitUntilReady(uint32_t timeout_ms) noexcept
{
    static constexpr uint32_t poll_interval_us = 10;
    const uint32_t max_cycles = (timeout_ms * 1000) / poll_interval_us;

    for (uint32_t i = 0; i < max_cycles; ++i)
    {
        Delay_us(poll_interval_us);

        uint8_t ac = 0;
        auto is_busy = IsBusy(ac);
        if (!is_busy)
        {
            return true;
        }
    }

    return false;
}

void LCD_TC1602A::SetupDataPins(IOMode mode) noexcept
{
    GPIO_InitTypeDef init{};
    init.Pull = GPIO_NOPULL;
    switch (mode)
    {
    case IOMode::Write:
        init.Mode = GPIO_MODE_OUTPUT_PP;
        init.Speed = GPIO_SPEED_FREQ_LOW;
        break;

    case IOMode::Read:
        init.Mode = GPIO_MODE_INPUT;
        break;
    }

    init.Pin = LCD_D2_Pin | LCD_D3_Pin | LCD_D6_Pin | LCD_D7_Pin;
    HAL_GPIO_Init(GPIOA, &init);

    init.Pin = LCD_D0_Pin | LCD_D1_Pin | LCD_D5_Pin;
    HAL_GPIO_Init(GPIOB, &init);

    init.Pin = LCD_D4_Pin;
    HAL_GPIO_Init(GPIOC, &init);
}

void LCD_TC1602A::SetRS(RegisterSelect rs) noexcept
{
    auto pin_state = [rs]() 
    {
        switch (rs)
        {
        case RegisterSelect::Instruction: return GPIO_PIN_RESET;
        case RegisterSelect::Data: return GPIO_PIN_SET;
        }
        __unreachable();
    }();
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, pin_state);
}

void LCD_TC1602A::SetIOMode(IOMode mode) noexcept
{
    auto pin_state = [mode]() 
    {
        switch (mode)
        {
        case IOMode::Write: return GPIO_PIN_RESET;
        case IOMode::Read: return GPIO_PIN_SET;
        }
        __unreachable();
    }();
    HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, pin_state);
}

void LCD_TC1602A::SetEnable(bool enable) noexcept
{
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LCD_TC1602A::SetData(data_t data) noexcept
{
    for (size_t i = 0; i < lcd_data_pins.size(); ++i)
    {
        auto& pin = lcd_data_pins[i];
        HAL_GPIO_WritePin(pin.first, pin.second, data.test(i) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

LCD_TC1602A::data_t LCD_TC1602A::ReadData() noexcept
{
    data_t data;
    for (size_t i = 0; i < lcd_data_pins.size(); ++i)
    {
        auto& pin = lcd_data_pins[i];
        auto state = HAL_GPIO_ReadPin(pin.first, pin.second);
        data.set(i, state != GPIO_PIN_RESET);
    }
    return data;
}

void LCD_TC1602A::SetupCommand(RegisterSelect rs, IOMode mode) noexcept
{
    SetRS(rs);
    Delay_100ns(1);
    SetIOMode(mode);
    Delay_100ns(1);
}

void LCD_TC1602A::SendWriteCommand(RegisterSelect rs, data_t data) noexcept
{
    static constexpr auto mode = IOMode::Write;
    static constexpr uint32_t max_command_time_ms = 5;  // Datasheet says 4.1ms max for clear display and return home.

    SetupDataPins(mode);
    SetupCommand(rs, mode);
    {
        AutoEnable auto_enable{ *this };
        SetData(data);
        Delay_100ns(2);	// Min tPW is 150ns.
    }
    Delay_100ns(2);	// Min cycle time is 400ns.

    WaitUntilReady(max_command_time_ms);
}

void LCD_TC1602A::SendWriteCommand(RegisterSelect rs, const std::span<uint8_t>& data) noexcept
{
    for (auto byte : data)
    {
        SendWriteCommand(rs, byte);
    }
}

LCD_TC1602A::data_t LCD_TC1602A::SendReadCommand(RegisterSelect rs) noexcept
{
    static constexpr auto mode = IOMode::Read;

    SetupDataPins(mode);
    SetupCommand(rs, mode);
    data_t data;
    {
        AutoEnable auto_enable{ *this };
        Delay_100ns(2); // Min pulse width is 150ns.
        data = ReadData();
    }
    Delay_100ns(2); // Min cycle time is 400ns.
    return data;
}