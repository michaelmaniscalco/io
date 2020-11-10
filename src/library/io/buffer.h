#pragma once

#include <include/endian.h>

#include <cstdint>
#include <memory>
#include <functional>


namespace maniscalco 
{

    class buffer final 
    {
    public:

        using size_type = std::size_t;
        using element_type = std::uint8_t;
        using iterator = element_type *;
        using const_iterator = element_type const *;

        buffer() = default;

        buffer(iterator, size_type, std::function<void(element_type *)> = nullptr);

        buffer(size_type);

        buffer(buffer &&);

        buffer & operator = (buffer &&);

        buffer(buffer const &) = delete;
        buffer & operator = (buffer const &) = delete;

        ~buffer() = default;

        size_type capacity() const;

        iterator begin() const;

        iterator end() const;

        element_type const * data() const;

        element_type * data();

        operator bool() const;

    private:

        size_type capacity_{0};

        std::unique_ptr<element_type [], std::function<void (element_type *)>> data_;

    }; // class buffer

}


//=============================================================================
inline maniscalco::buffer::operator bool
(
) const
{
    return (data_.get() != nullptr);
}


//=============================================================================
inline auto maniscalco::buffer::begin
(
) const -> iterator
{
    return data_.get();
}


//=============================================================================
inline auto maniscalco::buffer::end
(
) const -> iterator
{
    return begin() + capacity_;
}


//=============================================================================
inline auto maniscalco::buffer::data
(
) const -> element_type const *
{
    return data_.get();
}


//=============================================================================
inline auto maniscalco::buffer::data
(
) -> element_type *
{
    return data_.get();
}
