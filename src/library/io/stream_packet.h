#pragma once

#include "./buffer.h"
#include "./stream_direction.h"


namespace maniscalco::io
{

	template <stream_direction S>
    struct stream_packet
    {
    	using size_type = std::int64_t;

        stream_packet() = default;
        
        stream_packet
        (
            buffer && b, 
            size_type start, 
            size_type end
        ):
            buffer_(std::move(b)), 
            startOffset_(start), 
            endOffset_(end)
        {
        }

        stream_packet(stream_packet &&) = default;

        auto size() const
        {
            if constexpr (S == stream_direction::forward)
                return (endOffset_ - startOffset_);
            else
                return (startOffset_ - endOffset_);
        }

        auto data() const{return buffer_.data();}
        auto capacity() const{return buffer_.capacity();}
        using opposite_direction_packet = stream_packet<opposite_direction<S>::value>;

        stream_packet
        (
            opposite_direction_packet && other
        ):
            buffer_(std::move(other.buffer_)), 
            startOffset_(other.endOffset_), 
            endOffset_(other.startOffset_)
        {
        }

        buffer      buffer_;
        size_type   startOffset_;
        size_type   endOffset_;
    };

    using forward_stream_packet = stream_packet<stream_direction::forward>;
    using reverse_stream_packet = stream_packet<stream_direction::reverse>;

} // namespace maniscalco::io
