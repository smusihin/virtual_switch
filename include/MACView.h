#pragma once
#include <cstdint>
#include <cstring>
#include <ostream>
#include <iomanip>

class MACView
{
public:
    static constexpr size_t kSize = 6;

    explicit MACView(const uint8_t* data) : m_data(data) {}

    const uint8_t* data() const
    {
        return m_data;
    }

    const uint8_t& operator[](size_t i) const
    {
        return m_data[i];
    }

    bool operator==(const MACView& other) const
    {
        return std::memcmp(m_data, other.m_data, kSize) == 0;
    }

    bool operator!=(const MACView& other) const
    {
        return !(*this == other);
    }

private:
    const uint8_t* m_data;
};

inline std::ostream& operator<<(std::ostream& os, const MACView& mac) {
    auto flags = os.flags();

    os << std::hex << std::setfill('0');

    for (size_t i = 0; i < MACView::kSize; ++i) {
        os << std::setw(2) << static_cast<int>(mac[i]);
        if (i != MACView::kSize - 1)
            os << ":";
    }

    os.flags(flags);
    return os;
}

