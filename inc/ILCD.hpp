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
class ILCD
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