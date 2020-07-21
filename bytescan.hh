
#pragma once

#include "common.hh"

struct ByteScanner
{
    virtual ~ByteScanner() {}
    virtual byte_type consume() = 0;
    virtual std::size_t tell() const = 0;
    virtual std::size_t last() const { return SIZE_MAX; }
};

struct SpanScanner : public ByteScanner
{
    SpanScanner(byte_type const* begin, std::size_t length)
        : m_data(begin), m_offset(0), m_length(length) {}

    ~SpanScanner() override final {}

    inline byte_type consume() override final
    {
        if (m_offset == m_length)
            return 0; // TODO: throw std::logic_error?

        return m_data[m_offset++];
    }

    inline void seek(std::size_t offset)
    {
        m_offset = offset;
    }

    inline std::size_t tell() const override final
    {
        return m_offset;
    }

    inline std::size_t last() const override final
    {
        return m_length;
    }

    inline static SpanScanner from_vector(std::vector<byte_type> const& vector, std::size_t start, std::size_t length)
    {
        return SpanScanner(vector.data() + start, std::min(length, vector.size() - start));
    }

    inline static SpanScanner from_vector(std::vector<byte_type> const& vector)
    {
        return from_vector(vector, 0, vector.size());
    }

private:
    byte_type const* m_data;
    std::size_t m_offset;
    std::size_t m_length;
};
