#pragma once

#include <cstdint>


namespace maniscalco::io
{
	
	enum class stream_direction : std::int32_t
	{
		forward,
		reverse
	};


	template <stream_direction> struct opposite_direction;
	template <> struct opposite_direction<stream_direction::forward>{static auto constexpr value = stream_direction::reverse;};
	template <> struct opposite_direction<stream_direction::reverse>{static auto constexpr value = stream_direction::forward;};
}