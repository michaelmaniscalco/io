#pragma once

#include "./buffer.h"

#include <cstdint>
#include <functional>
#include <algorithm>
#include <memory>
#include <vector>

namespace maniscalco 
{

    class push_stream final 
    {
    public:

        static auto constexpr bits_per_byte = 8;
        using code_type = std::uint64_t;
        using size_type = std::size_t;

        static size_type constexpr default_buffer_size = (((1 << 10) * 2) * bits_per_byte);

        using buffer_allocation_handler = std::function<buffer()>;
        using buffer_output_handler = std::function<void(buffer, size_type)>;

        struct configuration_type 
        {
            buffer_output_handler bufferOutputHandler_;
            buffer_allocation_handler bufferAllocationHandler_;
        };

        push_stream() = default;

        push_stream(configuration_type const &);

        push_stream(push_stream &&);

        push_stream & operator = (push_stream &&);

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

    private:

        void flush_current_buffer();

        buffer_allocation_handler bufferAllocationHandler_;

        buffer buffer_;

        buffer::iterator writePosition_;

        buffer_output_handler bufferOutputHandler_;

        size_type size_{0};

        std::uint32_t internalBuffer_[2] = {0, 0};

        std::uint32_t internalSize_{0};

    }; // class push_stream

} // maniscalco


//=============================================================================
inline void maniscalco::push_stream::flush
(
)
{
    flush_current_buffer();
}


//=============================================================================
inline void maniscalco::push_stream::flush_current_buffer
(
)
{
    auto bits_to_flush = (internalSize_ + ((writePosition_ - buffer_.begin()) * bits_per_byte));
    if (bits_to_flush > 0)
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
        size_ += bits_to_flush;
        bufferOutputHandler_(std::move(buffer_), bits_to_flush);
        buffer_ = ((bufferAllocationHandler_) ? bufferAllocationHandler_() : buffer(default_buffer_size));
        writePosition_ = buffer_.begin();
    }
}


//=============================================================================
inline void maniscalco::push_stream::push
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
