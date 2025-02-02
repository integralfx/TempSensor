#include "LCD.hpp"
#include "Pins.hpp"
#include "Time.hpp"
#include "stm32l4xx_hal.h"
#include <array>
#include <utility>

bool ILCD::SendCommand(const LCDCommand& cmd)
{
    return m_send_cmd(m_ilcd_base_ptr, cmd);
}

LCD::LCD(ILCD& ilcd) : m_ilcd{ ilcd }
{

}

bool LCD::Init(const LCDInit& lcd_init)
{
    LCDCommand cmd
    {
        .register_select = LCDCommand::RegisterSelect::Instruction,
        .io_mode = LCDCommand::IOMode::Write,
        .data = static_cast<uint8_t>(
            0b100000 | (static_cast<uint8_t>(lcd_init.data_size) << 4)
                     | (static_cast<uint8_t>(lcd_init.row_count) << 3)
                     | (static_cast<uint8_t>(lcd_init.font_type) << 2)
        )
    };
    bool result = m_ilcd.SendCommand(cmd);
    if (result)
    {
        m_init = lcd_init;
    }
    return result;
}

LCDSettings LCD::GetSettings() const
{
    return m_settings;
}

bool LCD::SetSettings(const LCDSettings& lcd_settings)
{
    auto to_bit = [](bool b) -> uint8_t { return b ? 1 : 0; };
    LCDCommand cmd
    {
        .register_select = LCDCommand::RegisterSelect::Instruction,
        .io_mode = LCDCommand::IOMode::Write,
        .data = static_cast<uint8_t>(
            0b1000 | (to_bit(lcd_settings.display_on) << 2)
                   | (to_bit(lcd_settings.cursor_on) << 1)
                   | to_bit(lcd_settings.cursor_blink)
        )
    };
    bool result = m_ilcd.SendCommand(cmd);
    if (result)
    {
        m_settings = lcd_settings;
    }
    return result;
}

bool LCD::Clear()
{
    LCDCommand cmd
    {
        .register_select = LCDCommand::RegisterSelect::Instruction,
        .io_mode = LCDCommand::IOMode::Write,
        .data = 1
    };
    return m_ilcd.SendCommand(cmd);
}

bool LCD::Write(uint8_t data)
{
    LCDCommand cmd
    {
        .register_select = LCDCommand::RegisterSelect::Data,
        .io_mode = LCDCommand::IOMode::Write,
        .data = data
    };
    return m_ilcd.SendCommand(cmd);
}

bool LCD::SetCursor(uint8_t row, uint8_t col)
{
    if (row > static_cast<uint8_t>(m_init.row_count) || col > m_init.column_count)
    {
        return false;
    }

	uint8_t address = row * m_init.column_count + col;
	LCDCommand cmd
    {
        .register_select = LCDCommand::RegisterSelect::Instruction,
        .io_mode = LCDCommand::IOMode::Write,
        .data = static_cast<uint8_t>(0b10000000 | address)
    };
    return m_ilcd.SendCommand(cmd);
}

bool LCD::IsBusy(uint8_t& address_counter)
{
    return false;
}