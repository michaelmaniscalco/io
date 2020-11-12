#pragma once

#include "./buffer.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <tuple>


namespace maniscalco::io
{

    class pop_stream final 
    {
    public:

        using code_type = std::uint64_t;
        using size_type = std::size_t;
        using input_handler = std::function<std::tuple<buffer, size_type>()>;

        struct configuration_type 
        {
            input_handler inputHandler_;
        };

        pop_stream() = default;

        pop_stream(configuration_type const &);

        pop_stream(pop_stream &&);

        pop_stream& operator = (pop_stream &&);

        // pop_stream is non copyable - move only
        pop_stream(pop_stream const &) = delete;
        pop_stream & operator = (pop_stream const &) = delete;

        ~pop_stream() = default;

        code_type pop
        (
            size_type
        );

        code_type pop_bit();

        void discard
        (
            size_type
        );

        std::optional<code_type> peek
        (
            size_type
        ) const;

    private:

        code_type pop
        (
            size_type, 
            size_type
        ) const;

        void load_input_buffer();

        input_handler inputHandler_;

        buffer buffer_;

        size_type size_;

        size_type readPosition_;

    }; // class pop_stream

} 


//=============================================================================
inline void maniscalco::io::pop_stream::load_input_buffer
(
)
{
    auto [buffer, size] = inputHandler_();
    buffer_ = std::move(buffer);
    size_ = size;
    readPosition_ = 0;
}


//=============================================================================
inline void maniscalco::io::pop_stream::discard
(
    size_type count
)
{
    while (count > 0) 
    {
        auto available = (size_type)(size_ - readPosition_);
        if (available > count)
            available = count;
        readPosition_ += available;
        count -= available;
        if (readPosition_ == size_)
            load_input_buffer();
    }
}


//=============================================================================
inline auto maniscalco::io::pop_stream::pop_bit
(
) -> code_type
{
    if (readPosition_ >= size_)
        load_input_buffer();
    code_type result = ((buffer_.data()[readPosition_ >> 0x03] & (0x80 >> (readPosition_ & 0x07))) != 0);
    ++readPosition_;
    return result;
}


//=============================================================================
inline auto maniscalco::io::pop_stream::pop
(
    size_type code_length
) -> code_type
{
    auto next_read_position = (readPosition_ + code_length);
    if (next_read_position <= size_) 
    {
        auto code = pop(readPosition_, code_length);
        readPosition_ = next_read_position;
        return code;
    }
    else 
    {
        size_type n = (size_ - readPosition_);
        code_type code = (n > 0) ? code = pop(readPosition_, n) : 0;
        load_input_buffer();
        auto bits_remaining = (code_length - n);
        code <<= bits_remaining;
        code |= pop(readPosition_, bits_remaining);
        readPosition_ += bits_remaining;
        return code;
    }
}


//=============================================================================
inline auto maniscalco::io::pop_stream::pop
(
    size_type where, 
    size_type codeSize
) const -> code_type
{
    auto code = *(std::size_t *)(buffer_.data() + (where >> 0x03));
    code = endian_swap<network_order_type, host_order_type>(code);
    code >>= ((sizeof(std::size_t) << 3) - codeSize - (where & 0x07));
    return (code & ((1ull << codeSize) - 1));
}


//=============================================================================
inline auto maniscalco::io::pop_stream::peek
(
    size_type codeSize
) const -> std::optional<code_type>
{
    return ((readPosition_ + 32) <= size_) ? std::optional<code_type>(pop(readPosition_, codeSize)) : std::nullopt;
}
