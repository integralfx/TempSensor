#pragma once
#include <cstdint>

struct LCDCommand
{
    enum class RegisterSelect : uint8_t
    {
        Instruction,
        Data
    };

    enum class IOMode : uint8_t
    {
        Write,
        Read
    };

    RegisterSelect register_select;
    IOMode io_mode;
    uint8_t data;
};

struct LCDInit
{
    enum class DataSize : uint8_t
    {
        FourBits,
        EightBits
    };

    enum class Rows : uint8_t
    {
        One,
        Two
    };

    enum class FontType : uint8_t
    {
        FiveByEightDots,
        FiveByTenDots
    };

    DataSize data_size;
    Rows row_count;
    FontType font_type;
    uint8_t column_count;
};

struct LCDSettings
{
    bool display_on;
    bool cursor_on;
    bool cursor_blink;
};

// TODO: This also needs a Read() function for LCD::IsBusy()
template<typename T>
class ILCDBase
{
public:
    bool SendCommand(const LCDCommand& cmd)
    {
        return static_cast<T*>(this)->SendCommand(cmd);
    }
};

class ILCD
{
public:
    template<typename T>
    explicit ILCD(ILCDBase<T>& ilcd_base)
    {
        m_ilcd_base_ptr = &ilcd_base;
        m_send_cmd = +[](void* this_ptr, const LCDCommand& cmd)
        {
            return static_cast<ILCDBase<T>*>(this_ptr)->SendCommand(cmd);
        };
    }

    bool SendCommand(const LCDCommand& cmd);

private:
    void* m_ilcd_base_ptr;
    using send_cmd_fn = bool(*)(void*, const LCDCommand&);
    send_cmd_fn m_send_cmd;
};

class LCD
{
public:
    explicit LCD(ILCD& ilcd);

    bool Init(const LCDInit& lcd_init);
    LCDSettings GetSettings() const;
    bool SetSettings(const LCDSettings& lcd_settings);
    bool Clear();
    bool Write(uint8_t data);
    bool SetCursor(uint8_t row, uint8_t col);
    bool IsBusy(uint8_t& address_counter);

private:
    ILCD& m_ilcd;
    LCDInit m_init;
    LCDSettings m_settings;
};