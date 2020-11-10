#include "./buffer.h"


//=============================================================================
maniscalco::buffer::buffer
(
    // allocate using specified capacity
    size_type capacity
): 
    capacity_(capacity), 
    data_(new std::uint8_t[capacity_], [](auto p){delete [] p;})
{
}


//=============================================================================
maniscalco::buffer::buffer
(
    // seat over memory provided
    // optional deleter hook
    element_type * data, 
    size_type capacity, 
    std::function<void(element_type *)> deleter
): 
    capacity_(capacity), 
    data_(data, deleter)
{
}


//=============================================================================
maniscalco::buffer::buffer
(
    buffer && other
): 
    capacity_(other.capacity_), 
    data_(std::move(other.data_))
{
    other.capacity_ = 0;
    other.data_ = nullptr;
}


//=============================================================================
auto maniscalco::buffer::operator = 
(
    buffer && other
) -> buffer &
{
    capacity_ = other.capacity_;
    data_ = std::move(other.data_);
    other.data_ = nullptr;
    other.capacity_ = 0;
    return *this;
}


//=============================================================================
auto maniscalco::buffer::capacity
(
) const -> size_type
{
    return capacity_;
}

