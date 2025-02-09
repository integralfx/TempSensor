#include "ILCD.hpp"

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