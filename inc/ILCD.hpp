#pragma once
#include <cstdint>
#include <span>

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
    void Init(const LCDInit& init) noexcept
    {
        Impl().Init(init);
    }
    void SetSettings(const LCDSettings& settings) noexcept
    {
        Impl().SetSettings(settings);
    }
    void Clear()
    {
        Impl().Clear();
    }
    void SetAddress(uint8_t address) noexcept
    {
        Impl().SetAddress(address);
    }
    void Read(uint8_t& out_data) noexcept
    {
        Impl().Read(out_data);
    }
    void Write(uint8_t data) noexcept
    {
        Impl().Write(data);
    }
    [[nodiscard]] size_t WriteRow(const std::span<uint8_t>& data) noexcept
    {
        return Impl().WriteRow(data);
    }
    [[nodiscard]] bool IsBusy(uint8_t& address_counter) noexcept
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
    explicit ILCD(ILCDBase<T>& ilcd_base) noexcept
    {
        m_impl_ptr = &ilcd_base;
        static constexpr auto impl = +[](void* this_ptr) noexcept { return *static_cast<ILCDBase<T>*>(this_ptr); };
        m_init = +[](void* this_ptr, const LCDInit& init) noexcept { impl(this_ptr).Init(init); };
        m_set_settings = +[](void* this_ptr, const LCDSettings& settings) noexcept { impl(this_ptr).SetSettings(settings); };
        m_clear = +[](void* this_ptr) noexcept { impl(this_ptr).Clear(); };
        m_set_address = +[](void* this_ptr, uint8_t address) noexcept { impl(this_ptr).SetAddress(address); };
        m_read = +[](void* this_ptr, uint8_t& out_data) noexcept { impl(this_ptr).Read(out_data); };
        m_write = +[](void* this_ptr, uint8_t data) noexcept { impl(this_ptr).Write(data); };
        m_write_row = +[](void* this_ptr, const std::span<uint8_t>& data) noexcept { return impl(this_ptr).WriteRow(data); };
        m_is_busy = +[](void* this_ptr, uint8_t& address_counter) noexcept { return impl(this_ptr).IsBusy(address_counter); };
    }

    void Init(const LCDInit& init) noexcept;
    void SetSettings(const LCDSettings& settings) noexcept;
    void Clear() noexcept;
    void SetAddress(uint8_t address) noexcept;
    void Read(uint8_t& out_data) noexcept;
    void Write(uint8_t data) noexcept;
    [[nodiscard]] size_t WriteRow(const std::span<uint8_t>& data) noexcept;
    [[nodiscard]] bool IsBusy(uint8_t& address_counter) noexcept;

private:
    void* m_impl_ptr;
    using init_fn = void(*)(void*, const LCDInit&) noexcept;
    using set_settings_fn = void(*)(void*, const LCDSettings&) noexcept;
    using clear_fn = void(*)(void*) noexcept;
    using set_address_fn = void(*)(void*, uint8_t) noexcept;
    using read_fn = void(*)(void*, uint8_t&) noexcept;
    using write_fn = void(*)(void*, uint8_t) noexcept;
    using write_row_fn = size_t(*)(void*, const std::span<uint8_t>&) noexcept;
    using is_busy_fn = bool(*)(void*, uint8_t&) noexcept;
    init_fn m_init;
    set_settings_fn m_set_settings;
    clear_fn m_clear;
    set_address_fn m_set_address;
    read_fn m_read;
    write_fn m_write;
    write_row_fn m_write_row;
    is_busy_fn m_is_busy;
};