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

enum class LCDScrollDirection : bool
{
    Left,
    Right
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
    void Clear() noexcept
    {
        Impl().Clear();
    }
    void ReturnHome() noexcept
    {
        Impl().ReturnHome();
    }
    void SetAddress(uint8_t address) noexcept
    {
        Impl().SetAddress(address);
    }
    void SetCursor(uint8_t row, uint8_t col) noexcept
    {
        Impl().SetCursor(row, col);
    }
    void SetDisplayScroll(bool enable) noexcept
    {
        Impl().SetDisplayScroll(enable);
    }
    void SetDisplayScrollDirection(LCDScrollDirection dir) noexcept
    {
        Impl().SetDisplayScrollDirection(dir);
    }
    [[nodiscard]] uint8_t Read() noexcept
    {
        return Impl().Read();
    }
    [[nodiscard]] size_t Read(std::span<uint8_t> buffer) noexcept
    {
        return Impl().Read(buffer);
    }
    void Write(uint8_t data) noexcept
    {
        Impl().Write(data);
    }
    [[nodiscard]] size_t Write(const std::span<uint8_t>& data) noexcept
    {
        return Impl().Write(data);
    }
    [[nodiscard]] bool IsBusy(uint8_t& address_counter) noexcept
    {
        return Impl().IsBusy(address_counter);
    }

private:
    T& Impl() noexcept
    {
        return *static_cast<T*>(this);
    }
};