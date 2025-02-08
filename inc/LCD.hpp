#pragma once
#include <cstdint>

struct LCDInit
{
    enum class DataSize : bool
    {
        FourBits,
        EightBits
    };

    enum class Rows : bool
    {
        One,
        Two
    };

    enum class FontType : bool
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

template<typename T>
class ILCDBase
{
public:
    void Init(const LCDInit& init)
    {
        Impl().Init(init);
    }
    void SetSettings(const LCDSettings& settings)
    {
        Impl().SetSettings(settings);
    }
    void Clear()
    {
        Impl().Clear();
    }
    void SetAddress(uint8_t address)
    {
        Impl().SetAddress(address);
    }
    void Read(uint8_t& out_data)
    {
        Impl().Read(out_data);
    }
    void Write(uint8_t data)
    {
        Impl().Write(data);
    }
    bool IsBusy(uint8_t& address_counter)
    {
        return Impl().IsBusy(address_counter);
    }

private:
    T& Impl()
    {
        return *static_cast<T*>(this);
    }
};

class ILCD
{
public:
    template<typename T>
    explicit ILCD(ILCDBase<T>& ilcd_base)
    {
        m_impl_ptr = &ilcd_base;
        static constexpr auto impl = +[](void* this_ptr) { return *static_cast<ILCDBase<T>*>(this_ptr); };
        m_init = +[](void* this_ptr, const LCDInit& init) { impl(this_ptr).Init(init); };
        m_set_settings = +[](void* this_ptr, const LCDSettings& settings) { impl(this_ptr).SetSettings(settings); };
        m_clear = +[](void* this_ptr) { impl(this_ptr).Clear(); };
        m_set_address = +[](void* this_ptr, uint8_t address) { impl(this_ptr).SetAddress(address); };
        m_read = +[](void* this_ptr, uint8_t& out_data) { impl(this_ptr).Read(out_data); };
        m_write = +[](void* this_ptr, uint8_t data) { impl(this_ptr).Write(data); };
        m_is_busy = +[](void* this_ptr, uint8_t& address_counter) { return impl(this_ptr).IsBusy(address_counter); };
    }

    void Init(const LCDInit& init);
    void SetSettings(const LCDSettings& settings);
    void Clear();
    void SetAddress(uint8_t address);
    void Read(uint8_t& out_data);
    void Write(uint8_t data);
    bool IsBusy(uint8_t& address_counter);

private:
    void* m_impl_ptr;
    using init_fn = void(*)(void*, const LCDInit&);
    using set_settings_fn = void(*)(void*, const LCDSettings&);
    using clear_fn = void(*)(void*);
    using set_address_fn = void(*)(void*, uint8_t);
    using read_fn = void(*)(void*, uint8_t&);
    using write_fn = void(*)(void*, uint8_t);
    using is_busy_fn = bool(*)(void*, uint8_t&);
    init_fn m_init;
    set_settings_fn m_set_settings;
    clear_fn m_clear;
    set_address_fn m_set_address;
    read_fn m_read;
    write_fn m_write;
    is_busy_fn m_is_busy;
};

class LCD
{
public:
    explicit LCD(ILCD& ilcd);

    void Init(const LCDInit& lcd_init);
    LCDSettings GetSettings() const;
    void SetSettings(const LCDSettings& lcd_settings);
    void Clear();
    void SetAddress(uint8_t address);
    bool SetCursor(uint8_t row, uint8_t col);
    void Read(uint8_t& out_data);
    void Write(uint8_t data);
    bool IsBusy(uint8_t& address_counter);

private:
    ILCD& m_ilcd;
    LCDInit m_init;
    LCDSettings m_settings;
};