#include "./push_stream.h"


//=============================================================================
maniscalco::io::push_stream::push_stream
(
    configuration_type const & configuration
): 
    bufferAllocationHandler_(configuration.bufferAllocationHandler_),
    buffer_(bufferAllocationHandler_ ? bufferAllocationHandler_() : buffer(default_buffer_size)),
    writePosition_(buffer_.begin()),
    bufferOutputHandler_(configuration.bufferOutputHandler_),
    size_(0)
{
}


//=============================================================================
maniscalco::io::push_stream::push_stream
(
    push_stream && other
): 
    bufferAllocationHandler_(std::move(other.bufferAllocationHandler_)),
    buffer_(std::move(other.buffer_)),
    writePosition_(other.writePosition_),
    bufferOutputHandler_(std::move(other.bufferOutputHandler_)),
    size_(other.size_)
{
    other.bufferAllocationHandler_ = nullptr;
    other.buffer_ = buffer();
    other.bufferOutputHandler_ = nullptr;
    other.size_ = 0;
}


//=============================================================================
auto maniscalco::io::push_stream::operator =
(
    push_stream && other
) -> push_stream &
{
    bufferAllocationHandler_ = std::move(other.bufferAllocationHandler_);
    buffer_ = std::move(other.buffer_);
    writePosition_ = other.writePosition_;
    bufferOutputHandler_ = std::move(other.bufferOutputHandler_);
    size_ = other.size_;
    other.bufferAllocationHandler_ = nullptr;
    other.buffer_ = buffer();
    other.bufferOutputHandler_ = nullptr;
    other.size_ = 0;
    return *this;
}


//=============================================================================
maniscalco::io::push_stream::~push_stream
(
)
{
    flush();
}


//=============================================================================
auto maniscalco::io::push_stream::size
(
) const -> size_type
{
    return (size_ + ((writePosition_ - buffer_.begin()) * bits_per_byte) + internalSize_);
}
