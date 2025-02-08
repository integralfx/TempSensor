#include "LCD.hpp"
#include "Pins.hpp"
#include "Time.hpp"
#include "stm32l4xx_hal.h"
#include <array>
#include <utility>

void ILCD::Init(const LCDInit& init)
{
    m_init(m_impl_ptr, init);
}

void ILCD::SetSettings(const LCDSettings& settings)
{
    m_set_settings(m_impl_ptr, settings);
}

void ILCD::Clear()
{
    m_clear(m_impl_ptr);
}

void ILCD::SetAddress(uint8_t address)
{
    m_set_address(m_impl_ptr, address);
}

void ILCD::Read(uint8_t& out_data)
{
    m_read(m_impl_ptr, out_data);
}

void ILCD::Write(uint8_t data)
{
    m_write(m_impl_ptr, data);
}

bool ILCD::IsBusy(uint8_t& address_counter)
{
    return m_is_busy(m_impl_ptr, address_counter);
}

LCD::LCD(ILCD& ilcd) : m_ilcd{ ilcd }
{

}

void LCD::Init(const LCDInit& init)
{
    m_ilcd.Init(init);
    m_init = init;
}

LCDSettings LCD::GetSettings() const
{
    return m_settings;
}

void LCD::SetSettings(const LCDSettings& settings)
{
    m_ilcd.SetSettings(settings);
    m_settings = settings;
}

void LCD::Clear()
{
    m_ilcd.Clear();
}

void LCD::SetAddress(uint8_t address)
{
    m_ilcd.SetAddress(address);
}

bool LCD::SetCursor(uint8_t row, uint8_t col)
{
    auto row_count = [&]() -> uint8_t
    {
        switch (m_init.row_count)
        {
        case LCDInit::Rows::One: return 1;
        case LCDInit::Rows::Two: return 2;
        }
        __builtin_unreachable();
    }();

    if (row > row_count || col > m_init.column_count)
    {
        return false;
    }

    uint8_t address = row * m_init.column_count + row;
    SetAddress(0b10000000 | address);
    return true;
}

void LCD::Read(uint8_t& out_data)
{
    m_ilcd.Read(out_data);
}

void LCD::Write(uint8_t data)
{
    m_ilcd.Write(data);
}

bool LCD::IsBusy(uint8_t& address_counter)
{
    return m_ilcd.IsBusy(address_counter);
}