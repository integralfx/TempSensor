#pragma once
#include "LCD.hpp"
#include <bitset>

class LCD_TC1602A : public ILCD<LCD_TC1602A>
{
public:
    using data_t = std::bitset<8>;

    void Init(const LCDInit& init) noexcept;
    void SetSettings(const LCDSettings& settings) noexcept;
    void Clear() noexcept;
    void ReturnHome() noexcept;
    void SetAddress(LCDAddress type, uint8_t address) noexcept;
    void SetCursor(uint8_t row, uint8_t col) noexcept;
    void SetDisplayScroll(bool enable) noexcept;
    void SetDisplayScrollDirection(LCDScrollDirection dir) noexcept;
    [[nodiscard]] uint8_t Read() noexcept;
    [[nodiscard]] size_t Read(std::span<uint8_t> buffer) noexcept;
    void Write(uint8_t data) noexcept;
    [[nodiscard]] size_t Write(const std::span<uint8_t>& data) noexcept;
    [[nodiscard]] bool IsBusy(uint8_t& address_counter) noexcept;

private:
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

    enum class TextDirection : bool
    {
        RightToLeft,
        LeftToRight
    };

    enum class CommandIndex : size_t
    {
        ClearDisplay,
        ReturnHome,
        EntryMode,
        DisplayControl,
        CursorDisplayShift,
        Function
    };

    bool WaitUntilReady(uint32_t timeout_ms) noexcept;
    void SetEntryMode(TextDirection dir, bool enableDisplayScroll) noexcept;

    void SetupDataPins(IOMode mode) noexcept;

    void SetRS(RegisterSelect rs) noexcept;
    void SetIOMode(IOMode mode) noexcept;
    void SetEnable(bool enable) noexcept;

    void SetData(data_t data) noexcept;
    [[nodiscard]] data_t ReadData() noexcept;

    void SetupCommand(RegisterSelect rs, IOMode mode) noexcept;
    void SendWriteCommand(RegisterSelect rs, data_t data) noexcept;
    void SendWriteCommandAndWait(RegisterSelect rs, data_t data) noexcept;
    [[nodiscard]] data_t SendReadCommand(RegisterSelect rs) noexcept;

    friend class AutoEnable;
};