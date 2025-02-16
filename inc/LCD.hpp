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

    void ReturnHome() noexcept
    {
        m_ilcd.ReturnHome();
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

        m_ilcd.SetCursor(row, col);
        return true;
    }

    void SetDisplayScroll(bool enable) noexcept
    {
        m_ilcd.SetDisplayScroll(enable);
    }

    void SetDisplayScrollDirection(LCDScrollDirection dir) noexcept
    {
        m_ilcd.SetDisplayScrollDirection(dir);
    }

    [[nodiscard]] uint8_t Read() noexcept
    {
        return m_ilcd.Read();
    }

    [[nodiscard]] size_t Read(std::span<uint8_t> buffer) noexcept
    {
        return m_ilcd.Read(buffer);
    }

    void Write(uint8_t data) noexcept
    {
        m_ilcd.Write(data);
    }

    [[nodiscard]] size_t Write(const std::span<uint8_t>& data) noexcept
    {
        return m_ilcd.Write(data);
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