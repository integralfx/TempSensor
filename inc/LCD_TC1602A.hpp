#pragma once
#include "LCD.hpp"
#include <bitset>

class LCD_TC1602A : public ILCDBase<LCD_TC1602A>
{
public:
    using data_t = std::bitset<8>;

    void Init(const LCDInit& init) noexcept;
    void SetSettings(const LCDSettings& settings) noexcept;
    void Clear() noexcept;
    void SetAddress(uint8_t address) noexcept;
    void Read(uint8_t& out_data) noexcept;
    void Write(uint8_t data) noexcept;
    [[nodiscard]] size_t WriteRow(const std::span<uint8_t>& data) noexcept;
    [[nodiscard]] bool IsBusy(uint8_t& address_counter) noexcept;

private:
    bool WaitUntilReady(uint32_t timeout_ms) noexcept;

    enum class IOMode : uint8_t
    {
        Write,
        Read
    };

    enum class RegisterSelect : uint8_t
    {
        Instruction,
        Data
    };

    void SetupDataPins(IOMode mode) noexcept;

    void SetRS(RegisterSelect rs) noexcept;
    void SetIOMode(IOMode mode) noexcept;
    void SetEnable(bool enable) noexcept;

    void SetData(data_t data) noexcept;
    [[nodiscard]] data_t ReadData() noexcept;

    void SetupCommand(RegisterSelect rs, IOMode mode) noexcept;
    void SendWriteCommand(RegisterSelect rs, data_t data) noexcept;
    void SendWriteCommand(RegisterSelect rs, const std::span<uint8_t>& data) noexcept;
    [[nodiscard]] data_t SendReadCommand(RegisterSelect rs) noexcept;

    friend class AutoEnable;
};