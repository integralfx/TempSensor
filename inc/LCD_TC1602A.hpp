#pragma once
#include "LCD.hpp"
#include <bitset>

class LCD_TC1602A : public ILCDBase<LCD_TC1602A>
{
public:
    using data_t = std::bitset<8>;

    void Init(const LCDInit& init);
    void SetSettings(const LCDSettings& settings);
    void Clear();
    void SetAddress(uint8_t address);
    void Read(uint8_t& out_data);
    void Write(uint8_t data);
    bool IsBusy(uint8_t& address_counter);

private:
    bool WaitUntilReady(uint32_t timeout_ms);  // TODO: Add timeout

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

    void SetupDataPins(IOMode mode);

    void SetRS(RegisterSelect rs);
    void SetIOMode(IOMode mode);
    void SetEnable(bool enable);

    void SetData(data_t data);
    data_t ReadData();

    void SetupCommand(RegisterSelect rs, IOMode mode);
    void SendWriteCommand(RegisterSelect rs, data_t data);
    data_t SendReadCommand(RegisterSelect rs);
};