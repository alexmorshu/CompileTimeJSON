#pragma once
#include <cstddef>
#include "detail.hpp"

//TODO: Add error support

//compile-time string 
template <class T, T... chars>
struct Str
{
	static const std::size_t size = sizeof...(chars);

	template<class U, char... c>
	constexpr bool operator==(Str<U, c...>) const noexcept
	{
		return false;
	}
	constexpr bool operator==(Str) const noexcept
	{
		return true;
	}
	
	static void get(T* ptr) 
	{
		Write<T, chars...> g(ptr);	
	}
	
};

//GNU specific
template<class T, T... chars>
Str<T, chars...> operator""_GCT()
{
	return {};
}









