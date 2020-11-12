#pragma once

#include "./buffer.h"
#include "./stream_direction.h"
#include "./stream_packet.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <tuple>


namespace maniscalco::io
{

    template <stream_direction S>
    class pop_stream final 
    {
    public:

        static auto constexpr bits_per_byte = 8;

        using code_type = std::uint64_t;
        using size_type = std::int64_t;
        using packet_type = stream_packet<S>;
        using input_handler = std::function<packet_type()>;

        struct configuration_type 
        {
            input_handler inputHandler_;
        };

        pop_stream() = default;

        pop_stream(configuration_type const &);

        pop_stream(pop_stream &&) = default;

        pop_stream& operator = (pop_stream &&) = default;

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

        size_type size_consumed() const;

        void align();

    private:

        code_type pop
        (
            size_type, 
            size_type
        ) const;

        void load_input_buffer();

        input_handler inputHandler_;

        buffer buffer_;

        size_type endCurrentBuffer_{0};

        size_type beginCurrentBuffer_{0};

        size_type readPosition_{0};

        size_type maxSafePeekPosition_{0};

        size_type sizeConsumed_{0};

    }; // class pop_stream

    using forward_pop_stream = pop_stream<stream_direction::forward>;
    using reverse_pop_stream = pop_stream<stream_direction::reverse>;

} // namespace maniscalco::io


//=============================================================================
template <maniscalco::io::stream_direction S>
auto maniscalco::io::pop_stream<S>::size_consumed
(
    // returns the number of bits consumed by this stream thus far
) const -> size_type
{
    return (sizeConsumed_ + (readPosition_ - beginCurrentBuffer_));
}


//=============================================================================
template <maniscalco::io::stream_direction S>
inline void maniscalco::io::pop_stream<S>::load_input_buffer
(
)
{
    sizeConsumed_ += (readPosition_ - beginCurrentBuffer_);
    stream_packet<S> packet = inputHandler_();
    buffer_ = std::move(packet.buffer_);
    endCurrentBuffer_ = packet.endOffset_;
    readPosition_ = beginCurrentBuffer_ = packet.startOffset_;
    maxSafePeekPosition_ = ((buffer_.capacity() * bits_per_byte) - 32);
}



//=============================================================================
template <>
inline void maniscalco::io::forward_pop_stream::discard
(
    size_type count
)
{
    while (count > 0) 
    {
        auto available = (size_type)(endCurrentBuffer_ - readPosition_);
        if (available > count)
            available = count;
        readPosition_ += available;
        count -= available;
        if (readPosition_ == endCurrentBuffer_)
            load_input_buffer();
    }
}


//=============================================================================
template <>
inline void maniscalco::io::forward_pop_stream::align
(
    // discard bits until at next byte bounardy
)
{
    if (readPosition_ & 0x07)
    {
        auto n = (8 - (readPosition_ & 0x07));
        discard(n);
    }
}


//=============================================================================
template <>
inline void maniscalco::io::reverse_pop_stream::discard
(
    size_type count
)
{
    // TODO:
    while (count > 0) 
    {
        auto available = (size_type)(readPosition_ - endCurrentBuffer_);
        if (available > count)
            available = count;
        readPosition_ -= available;
        count -= available;
        if (readPosition_ == endCurrentBuffer_)
            load_input_buffer();
    }
}


//=============================================================================
template <>
inline auto maniscalco::io::forward_pop_stream::pop_bit
(
) -> code_type
{
    if (readPosition_ >= endCurrentBuffer_)
        load_input_buffer();
    code_type result = ((buffer_.data()[readPosition_ >> 0x03] & (0x80 >> (readPosition_ & 0x07))) != 0);
    ++readPosition_;
    return result;
}


//=============================================================================
template <>
inline auto maniscalco::io::reverse_pop_stream::pop_bit
(
) -> code_type
{
    // TODO:
    if (--readPosition_ < endCurrentBuffer_)
        load_input_buffer();
    code_type result = ((buffer_.data()[readPosition_ >> 0x03] & (0x80 >> (readPosition_ & 0x07))) != 0);
    return result;
}


//=============================================================================
template <maniscalco::io::stream_direction S>
inline auto maniscalco::io::pop_stream<S>::pop
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
template <>
inline auto maniscalco::io::forward_pop_stream::pop
(
    size_type codeLength
) -> code_type
{
    auto nextReadPosition = (readPosition_ + codeLength);
    if (nextReadPosition <= endCurrentBuffer_) 
    {
        auto code = pop(readPosition_, codeLength);
        readPosition_ = nextReadPosition;
        return code;
    }
    else 
    {
        size_type n = (endCurrentBuffer_ - readPosition_);
        code_type code = (n > 0) ? code = pop(readPosition_, n) : 0;
        load_input_buffer();
        auto bits_remaining = (codeLength - n);
        code <<= bits_remaining;
        code |= pop(readPosition_, bits_remaining);
        readPosition_ += bits_remaining;
        return code;
    }
}


//=============================================================================
template <>
inline auto maniscalco::io::reverse_pop_stream::pop
(
    size_type codeLength
) -> code_type
{
    // TODO
    auto nextReadPosition = (readPosition_ - codeLength);
    if (nextReadPosition >= endCurrentBuffer_) 
    {
        auto code = pop(readPosition_ = nextReadPosition, codeLength);
        return code;
    }
    else 
    {
        size_type n = (readPosition_ -= endCurrentBuffer_);
        code_type code = (n > 0) ? code = pop(endCurrentBuffer_, n) : 0;
        load_input_buffer();
        auto bits_remaining = (codeLength - n);
        code <<= bits_remaining;
        readPosition_ -= bits_remaining;
        code |= pop(readPosition_, bits_remaining);
        return code;
    }
}


//=============================================================================
template <>
inline auto maniscalco::io::forward_pop_stream::peek
(
    size_type codeSize
) const -> std::optional<code_type>
{
    return (readPosition_ <= maxSafePeekPosition_) ? 
            std::optional<code_type>(pop(readPosition_, codeSize)) : std::nullopt;
}


//=============================================================================
template <>
inline auto maniscalco::io::reverse_pop_stream::peek
(
    size_type codeSize
) const -> std::optional<code_type>
{
    return ((codeSize <= readPosition_) && (readPosition_ <= maxSafePeekPosition_)) ? 
            std::optional<code_type>(pop(readPosition_ - codeSize, codeSize)) : std::nullopt;
}
