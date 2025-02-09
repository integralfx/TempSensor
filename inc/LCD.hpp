#pragma once
#include "ILCD.hpp"
#include <cstdint>
#include <span>

class LCD
{
public:
    explicit LCD(ILCD& ilcd) noexcept;

    void Init(const LCDInit& lcd_init) noexcept;
    LCDSettings GetSettings() const noexcept;
    void SetSettings(const LCDSettings& lcd_settings) noexcept;
    void Clear() noexcept;
    void SetAddress(uint8_t address) noexcept;
    [[nodiscard]] bool SetCursor(uint8_t row, uint8_t col) noexcept;
    void Read(uint8_t& out_data) noexcept;
    void Write(uint8_t data) noexcept;
    size_t WriteRow(const std::span<uint8_t>& data) noexcept;
    [[nodiscard]] bool IsBusy(uint8_t& address_counter) noexcept;

private:
    ILCD& m_ilcd;
    LCDInit m_init;
    LCDSettings m_settings;

    [[nodiscard]] size_t GetRowCount() const noexcept;
};