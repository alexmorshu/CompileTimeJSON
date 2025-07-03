#pragma once
#include <cstddef>
#include <array>
#include <iostream>
#include <tuple>
#include <vector>
#include <algorithm>

template< class... >
using void_t = void;

template<class T, T... value>
struct Write;

template <class T, T f, T... value>
struct Write<T, f, value...>
{
	Write(T* ptr) noexcept
	{
		*ptr = f;
		Write<T, value...> (ptr+1);
	}
};


template <class T>
struct Write<T>
{
	constexpr Write(T* ptr) noexcept
	{}
};


template <class T, class U = void>
struct ValueExists
{
	static const bool value = false;
};

template<class T>
struct ValueExists<T, void_t<decltype(std::declval<T>().value)>>
{
	static const bool value = true;
};



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
	
	constexpr void get(T* ptr) const 
	{
		Write<T, chars...> g(ptr);	
	}
	
};


template<class Key, class... JSONelem>
struct JSONBranch
{
	using StrKey = Key;

	std::tuple<JSONelem...> values;
	
	template<class... T>
	struct SumSize
	{
		static const std::size_t value = 0;	
	};
	
	template<class F, class... T>
	struct SumSize<F, T...>
	{
		static const std::size_t value = F::size + SumSize<T...>::value;	
	};



	static const std::size_t size = SumSize<JSONelem...>::value;
	
	template <std::size_t n, class Find, class... JSONelems>
	struct Get;

	template <std::size_t n, class Find, class f, class... JSONelems>
	struct Get<n, Find, f, JSONelems...>
	{
		static const std::size_t i = std::conditional<std::is_same<Find,typename f::StrKey>::value, std::integral_constant<std::size_t, n>, Get<n+1, Find, JSONelems...>>::type::value;
	};

	template<class Find>
	struct typeJson
	{
		using type = decltype(std::get<Get<0, Find, JSONelem...>::i>(values));
	};

	template<class Find>
	struct ret
	{
		using type = typename std::conditional<
		ValueExists<typename typeJson<Find>::type>::value,
		decltype(std::declval<typename typeJson<Find>::type>().value),
		typename typeJson<Find>::type>::type;
	};

	template<class T>
	constexpr typename ret<T>::type& operator[](T str)
	{
		constexpr std::size_t s = Get<0, T, JSONelem...>::i;
		return static_cast<typename ret<T>::type&>(std::get<s>(values));
	}
	template<class T>
	constexpr typename std::add_const<typename ret<T>::type>::type& operator[](T str) const
	{
		constexpr std::size_t s = Get<0, T, JSONelem...>::i;
		return static_cast<const typename ret<T>::type&>(std::get<s>(values));
	}
};


template<class Key, class ValueType>
struct JSONBaseElem
{
	static const std::size_t size = 1;
	using StrKey = Key;
	ValueType value;
	constexpr operator ValueType&()
	{
		return value;
	}
	constexpr operator const ValueType&() const
	{
		return value;
	}
};

template<class Key, class ValueType>
struct JSONLeaf: JSONBaseElem<Key, ValueType> 
{
};


template<class Key, class ValueType, std::size_t size>
struct JSONArray: JSONBaseElem<Key, ValueType[size]>
{

};


template<class Key, class ValueType>
struct JSONArray<Key, ValueType, static_cast<std::size_t>(-1)> : JSONBaseElem<Key, std::vector<ValueType>>
{
};

template<class T, T... chars>
Str<T, chars...> operator""_GCT()
{
	return {};
}







int main()
{
	JSONBranch<decltype("root"_GCT), JSONArray<decltype("name"_GCT), double, 2>, JSONArray<decltype("password"_GCT), double, 2>> a;	
	const decltype(a)& b = a;
	const double (&arr)[2] = b["name"_GCT];
	a["name"_GCT][1] = 32.2;
	std::cout << decltype(a)::size << std::endl;
}
