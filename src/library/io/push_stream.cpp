#include "./push_stream.h"


//=============================================================================
template <>
maniscalco::io::forward_push_stream::push_stream
(
    configuration_type const & configuration
): 
    bufferAllocationHandler_(configuration.bufferAllocationHandler_ ? configuration.bufferAllocationHandler_ : 
            [](){return buffer(default_buffer_size);}),
    buffer_(bufferAllocationHandler_()),
    writePosition_(buffer_.begin()),
    bufferOutputHandler_(configuration.bufferOutputHandler_),
    size_(0),
    internalSize_(0),
    internalBuffer_{0,0}
{
}


//=============================================================================
template <>
maniscalco::io::reverse_push_stream::push_stream
(
    configuration_type const & configuration
): 
    bufferAllocationHandler_(configuration.bufferAllocationHandler_ ? configuration.bufferAllocationHandler_ : 
            [](){return buffer(default_buffer_size);}),
    buffer_(bufferAllocationHandler_()),
    writePosition_(buffer_.end()),
    bufferOutputHandler_(configuration.bufferOutputHandler_),
    size_(0),
    internalSize_(0),
    internalBuffer_{0,0}
{
}


//=============================================================================
template <maniscalco::io::stream_direction S>
maniscalco::io::push_stream<S>::~push_stream
(
)
{
    flush();
}


//=============================================================================
template <>
auto maniscalco::io::forward_push_stream::size
(
) const -> size_type
{
    return (size_ + ((writePosition_ - buffer_.begin()) * bits_per_byte) + internalSize_);
}


//=============================================================================
template <>
auto maniscalco::io::reverse_push_stream::size
(
) const -> size_type
{
    return (size_ + ((buffer_.end() - writePosition_) * bits_per_byte) + internalSize_);
}


//=============================================================================
namespace maniscalco::io
{
    template class push_stream<stream_direction::forward>;
    template class push_stream<stream_direction::reverse>;

} // maniscalco