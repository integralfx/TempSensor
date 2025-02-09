#include "LCD.hpp"
#include "Pins.hpp"
#include "Time.hpp"
#include "stm32l4xx_hal.h"
#include <array>
#include <utility>

void ILCD::Init(const LCDInit& init) noexcept
{
    m_init(m_impl_ptr, init);
}

void ILCD::SetSettings(const LCDSettings& settings) noexcept
{
    m_set_settings(m_impl_ptr, settings);
}

void ILCD::Clear() noexcept
{
    m_clear(m_impl_ptr);
}

void ILCD::SetAddress(uint8_t address) noexcept
{
    m_set_address(m_impl_ptr, address);
}

void ILCD::Read(uint8_t& out_data) noexcept
{
    m_read(m_impl_ptr, out_data);
}

void ILCD::Write(uint8_t data) noexcept
{
    m_write(m_impl_ptr, data);
}

size_t ILCD::WriteRow(const std::span<uint8_t>& data) noexcept
{
    return m_write_row(m_impl_ptr, data);
}

bool ILCD::IsBusy(uint8_t& address_counter) noexcept
{
    return m_is_busy(m_impl_ptr, address_counter);
}

LCD::LCD(ILCD& ilcd) noexcept : m_ilcd{ ilcd }
{

}

void LCD::Init(const LCDInit& init) noexcept
{
    m_ilcd.Init(init);
    m_init = init;
}

LCDSettings LCD::GetSettings() const noexcept
{
    return m_settings;
}

void LCD::SetSettings(const LCDSettings& settings) noexcept
{
    m_ilcd.SetSettings(settings);
    m_settings = settings;
}

void LCD::Clear() noexcept
{
    m_ilcd.Clear();
}

void LCD::SetAddress(uint8_t address) noexcept
{
    m_ilcd.SetAddress(address);
}

bool LCD::SetCursor(uint8_t row, uint8_t col) noexcept
{
    auto row_count = GetRowCount();
    if (row >= row_count || col >= m_init.column_count)
    {
        return false;
    }

    uint8_t address = row * m_init.column_count + row;
    SetAddress(0b10000000 | address);
    return true;
}

void LCD::Read(uint8_t& out_data) noexcept
{
    m_ilcd.Read(out_data);
}

void LCD::Write(uint8_t data) noexcept
{
    m_ilcd.Write(data);
}

size_t LCD::WriteRow(const std::span<uint8_t>& data) noexcept
{
    uint8_t address_counter;
    UNUSED(IsBusy(address_counter));
    auto total_cells = GetRowCount() * m_init.column_count;
    auto available_cells = total_cells - address_counter;
    auto size = std::min(data.size(), available_cells);
    return m_ilcd.WriteRow(data.subspan(0, size));
}

bool LCD::IsBusy(uint8_t& address_counter) noexcept
{
    return m_ilcd.IsBusy(address_counter);
}

size_t LCD::GetRowCount() const noexcept
{
    switch (m_init.row_count)
    {
    case LCDInit::Rows::One: return 1;
    case LCDInit::Rows::Two: return 2;
    }
    return 0;
}