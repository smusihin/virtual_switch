#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

class EthHeaderView
{
public:
    static constexpr size_t kHeaderSize = 14;

    explicit EthHeaderView(std::span<uint8_t> buffer)
        : m_data(buffer.data()), m_len(buffer.size()) {}

    bool valid() const
    {
        return m_len >= kHeaderSize;
    }

    const uint8_t* dst() const
    {
        return m_data;
    }

    const uint8_t* src() const
    {
        return m_data + 6;
    }

    uint16_t ethertype() const
    {
        return (m_data[12] << 8) | m_data[13];
    }

    bool hasVlan() const
    {
        return ethertype() == 0x8100;
    }

    std::span<const uint8_t> payload() const
    {
        return std::span(m_data + kHeaderSize, m_len - kHeaderSize);
    }

private:
    const uint8_t* m_data;
    size_t m_len;
};
