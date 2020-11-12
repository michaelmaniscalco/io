#include "./pop_stream.h"


//=============================================================================
maniscalco::io::pop_stream::pop_stream
(
    configuration_type const & configuration
): 
    inputHandler_(configuration.inputHandler_), 
    buffer_(), 
    size_(0), 
    readPosition_(0)
{
}


//=============================================================================
maniscalco::io::pop_stream::pop_stream
(
    pop_stream && other
): 
    inputHandler_(std::move(other.inputHandler_)),
    buffer_(std::move(other.buffer_)),
    size_(other.size_),
    readPosition_(other.readPosition_)
{
    other.inputHandler_ = nullptr;
    other.buffer_ = buffer();
}


//=============================================================================
auto maniscalco::io::pop_stream::operator =
(
    pop_stream && other
) -> pop_stream &
{
    inputHandler_ = std::move(other.inputHandler_);
    buffer_ = std::move(other.buffer_);
    size_ = other.size_;
    readPosition_ = other.readPosition_;
    other.inputHandler_ = nullptr;
    other.buffer_ = buffer();
    return *this;
}
