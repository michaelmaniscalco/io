#pragma once

#include "./buffer.h"
#include "./stream_direction.h"
#include "./stream_packet.h"

#include <cstdint>
#include <functional>
#include <algorithm>
#include <memory>
#include <vector>
#include <tuple>


namespace maniscalco::io
{

    template <stream_direction S>
    class push_stream final 
    {
    public:

        static auto constexpr bits_per_byte = 8;
        using code_type = std::uint64_t;
        using size_type = std::int64_t;
        using packet_type = stream_packet<S>;

        static size_type constexpr default_buffer_size = ((1 << 10) * 8);


        using buffer_allocation_handler = std::function<buffer()>;
        using buffer_output_handler = std::function<void(packet_type)>;

        struct configuration_type 
        {
            buffer_output_handler bufferOutputHandler_;
            buffer_allocation_handler bufferAllocationHandler_;
        };

        push_stream() = default;

        push_stream(configuration_type const &);

        push_stream(push_stream &&) = default;

        push_stream & operator = (push_stream &&) = default;

        // push_stream is non-copyable - move only
        push_stream(push_stream const &) = delete;
        push_stream & operator = (push_stream const &) = delete;

        ~push_stream();

        void push
        (
            code_type, 
            size_type
        );

        size_type size() const;

        void flush();

        void align();

    private:

        void flush_current_buffer();

        buffer_allocation_handler bufferAllocationHandler_;

        buffer buffer_;

        buffer::iterator writePosition_;

        buffer_output_handler bufferOutputHandler_;

        size_type size_{0};

        std::uint32_t internalBuffer_[2] = {0, 0};

        size_type internalSize_{0};

    }; // class push_stream


    using forward_push_stream = push_stream<stream_direction::forward>;
    using reverse_push_stream = push_stream<stream_direction::reverse>;

} // maniscalco::io


//=============================================================================
template <maniscalco::io::stream_direction S>
inline void maniscalco::io::push_stream<S>::flush
(
)
{
    flush_current_buffer();
}


//=============================================================================
template <>
inline void maniscalco::io::forward_push_stream::flush_current_buffer
(
)
{
    auto bitsToFlush = (internalSize_ + ((writePosition_ - buffer_.begin()) * bits_per_byte));
    if (bitsToFlush > 0)
    {
        if (internalSize_ > 0)
        {
            // ensure that any internally buffered bits are also flushed
            using output_type = std::uint32_t;
            *(output_type *)(writePosition_) = internalBuffer_[0];
            internalBuffer_[0] = 0x00;
            internalBuffer_[1] = 0x00;
            internalSize_ = 0;
        }
        size_ += bitsToFlush;
        bufferOutputHandler_({std::move(buffer_), 0, bitsToFlush});
        buffer_ = bufferAllocationHandler_();
        writePosition_ = buffer_.begin();
    }
}


//=============================================================================
template <>
inline void maniscalco::io::reverse_push_stream::flush_current_buffer
(
)
{
    auto bitsToFlush = (internalSize_ + ((buffer_.end() - writePosition_) * bits_per_byte));
    if (bitsToFlush > 0)
    {
        if (internalSize_ > 0)
        {
            // ensure that any internally buffered bits are also flushed
            using output_type = std::uint32_t;
            writePosition_ -= sizeof(output_type);
            *(output_type *)(writePosition_) = internalBuffer_[1];
            internalBuffer_[0] = 0x00;
            internalBuffer_[1] = 0x00;
            internalSize_ = 0;
        }
        size_ += bitsToFlush;
        auto bufferEndOffset = buffer_.capacity() * bits_per_byte;
        bufferOutputHandler_({std::move(buffer_), bufferEndOffset, bufferEndOffset - bitsToFlush});
        buffer_ = bufferAllocationHandler_();
        writePosition_ = buffer_.end();
    }
}


//=============================================================================
template <>
inline void maniscalco::io::forward_push_stream::push
(
    // max codeSize = 32
    code_type code, 
    size_type codeSize
)
{
    code <<= (64 - codeSize - internalSize_);
    code = endian_swap<host_order_type, network_order_type>(code);
    *(std::size_t *)(internalBuffer_) |= code;
    internalSize_ += codeSize;
    if (internalSize_ >= 32)
    {
        // TODO: what if write position + sizeof(output_type) > endWritePosition_ ??
        using output_type = std::uint32_t;
        *(output_type *)(writePosition_) = internalBuffer_[0];
        internalBuffer_[0] = internalBuffer_[1];
        internalBuffer_[1] = 0x00;
        writePosition_ += sizeof(output_type);
        internalSize_ -= (sizeof(output_type) * bits_per_byte);
        if (writePosition_ >= buffer_.end())
        {
            // hack hides any remaining 'internal bits' during flush. figure out cleaner way
            auto temp = internalSize_;
            internalSize_ = 0;
            flush_current_buffer();
            internalSize_ = temp;
        }
    }
}


//=============================================================================
template <>
inline void maniscalco::io::reverse_push_stream::push
(
    // max codeSize = 32
    code_type code, 
    size_type codeSize
)
{
    code <<= internalSize_;
    code = endian_swap<host_order_type, network_order_type>(code);
    *(std::size_t *)(internalBuffer_) |= code;
    if ((internalSize_ += codeSize) >= 32)
    {
        // TODO: what if write position is < sizeof(output_type) ??
        using output_type = std::uint32_t;
        writePosition_ -= sizeof(output_type);
        *(output_type *)(writePosition_) = internalBuffer_[1];
        internalBuffer_[1] = internalBuffer_[0];
        internalBuffer_[0] = 0x00;
        internalSize_ -= (sizeof(output_type) * bits_per_byte);
        if (writePosition_ <= buffer_.begin())
        {
            // hack hides any remaining 'internal bits' during flush. figure out cleaner way
            auto temp = internalSize_;
            internalSize_ = 0;
            flush_current_buffer();
            internalSize_ = temp;
        }
    }
}


//=============================================================================
template <>
inline void maniscalco::io::reverse_push_stream::align
(
    // align bit stream to next byte boundary
)
{
    if (internalSize_ & 0x07)
    {
        auto n = (8 - internalSize_ & 0x07);
        push(0, n);
    }
}
