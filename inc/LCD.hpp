#pragma once
#include "ILCD.hpp"
#include <cstdint>
#include <span>

template<typename T>
class LCD
{
public:
    explicit LCD(ILCD<T>& ilcd) noexcept : m_ilcd{ ilcd }
    {

    }

    void Init(const LCDInit& init) noexcept
    {
        m_ilcd.Init(init);
        m_init = init;
    }

    [[nodiscard]] LCDSettings GetSettings() const noexcept
    {
        return m_settings;
    }

    void SetSettings(const LCDSettings& settings) noexcept
    {
        m_ilcd.SetSettings(settings);
        m_settings = settings;
    }

    void Clear() noexcept
    {
        m_ilcd.Clear();
    }

    void SetAddress(uint8_t address) noexcept
    {
        m_ilcd.SetAddress(address);
    }

    [[nodiscard]] bool SetCursor(uint8_t row, uint8_t col) noexcept
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

    void Read(uint8_t& out_data) noexcept
    {
        m_ilcd.Read(out_data);
    }

    void Write(uint8_t data) noexcept
    {
        m_ilcd.Write(data);
    }

    [[nodiscard]] size_t WriteRow(const std::span<uint8_t>& data) noexcept
    {
        uint8_t address_counter;
        UNUSED(IsBusy(address_counter));
        auto total_cells = GetRowCount() * m_init.column_count;
        auto available_cells = total_cells - address_counter;
        auto size = std::min(data.size(), available_cells);
        return m_ilcd.WriteRow(data.subspan(0, size));
    }

    [[nodiscard]] bool IsBusy(uint8_t& address_counter) noexcept
    {
        return m_ilcd.IsBusy(address_counter);
    }

private:
    ILCD<T>& m_ilcd;
    LCDInit m_init;
    LCDSettings m_settings;

    [[nodiscard]] size_t GetRowCount() const noexcept
    {
        switch (m_init.row_count)
        {
        case LCDInit::Rows::One: return 1;
        case LCDInit::Rows::Two: return 2;
        }
        return 0;
    }
};