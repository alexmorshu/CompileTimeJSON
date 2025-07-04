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
	
	static constexpr void get(T* ptr) 
	{
		Write<T, chars...> g(ptr);	
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


//just value
template<class Key, class ValueType>
struct JSONLeaf: JSONBaseElem<Key, ValueType> 
{
};

//fix array
template<class Key, class ValueType, std::size_t size>
struct JSONArray: JSONBaseElem<Key, ValueType[size]>
{

};

//dynamic array
template<class Key, class ValueType>
struct JSONArray<Key, ValueType, static_cast<std::size_t>(-1)> : JSONBaseElem<Key, std::vector<ValueType>>
{
};

template<class Key, class... JSONelem>
struct JSONBranch
{
	using StrKey = Key;

	std::tuple<JSONelem...> values;
	

	//get count of elem
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
	//count of elems	
	static const std::size_t size = SumSize<JSONelem...>::value;
	


	//contain indexs for tuple to get elem in seq
	template<std::size_t... nums>
	struct NumsContainer
	{};

	//detail for GetSeq
	template<std::size_t currentSize, std::size_t index, class JSONType>
	struct ContainerForGetSeg
	{
		static const std::size_t size = currentSize;
		static const std::size_t i = index;
		using type = JSONType;
	};

	//get next index 
	template<std::size_t n, std::size_t current, std::size_t index, bool end, class... JSONelems>
	struct GetSeq
	{
		using value = void;	
	};

	template<std::size_t n,  std::size_t index, class f, class... JSONelems>
	struct GetSeq<n, n, index, false,f, JSONelems...>
	{
		using value = ContainerForGetSeg<n, index, f>;
	};

	template<std::size_t n, std::size_t current, std::size_t index, class f, class... JSONelems>
	struct GetSeq<n, current, index,false,f, JSONelems...>
	{;
		using value = typename std::conditional<
				/*if*/ ((current + f::size) > n) ,
				/*then*/ ContainerForGetSeg<current, index, f>,
				/*else*/ typename GetSeq<n, current+f::size, index+1, ((current + f::size) > n) , JSONelems...>::value
					>::type;
	};


	//get NumsContainer for num
	template<std::size_t n, std::size_t current, bool isElem, std::size_t... indexs>
	struct Test
	{
		using value = typename GetSeq<n, current, 0, false ,JSONelem...>::value;
		using contain_type = NumsContainer<indexs..., value::i>;
	};

	template<std::size_t n, std::size_t current, std::size_t... indexs>
	struct Test<n, current, ValueExists<typename GetSeq<n, current, 0, false ,JSONelem...>::value::type>::value, indexs...>
	{
		using value = typename GetSeq<n, current, 0, false ,JSONelem...>::value;
		using contain_type = typename value::type::Test<n, value::size, false ,indexs..., value::i>::contain_type;
	};





	//Get num of type with key=Find
	template <std::size_t n, class Find, class... JSONelems>
	struct Get;

	template <std::size_t n, class Find, class f, class... JSONelems>
	struct Get<n, Find, f, JSONelems...>
	{
		static const std::size_t i = std::conditional<std::is_same<Find,typename f::StrKey>::value, std::integral_constant<std::size_t, n>, Get<n+1, Find, JSONelems...>>::type::value;
	};


	//get type of json (branch/elem)
	template<class Find>
	struct typeJson
	{
		using type = decltype(std::get<Get<0, Find, JSONelem...>::i>(values));
	};


	//get type value, if it's a elem or branch
	template<class Find>
	struct ret
	{
		using type = typename std::conditional<
			ValueExists<typename typeJson<Find>::type>::value,
			decltype(std::declval<typename typeJson<Find>::type>().value),
			typename typeJson<Find>::type
		>::type;
	};


	//get ref of value if it`s a elem with key str, or ref to branch
	template<class T>
	constexpr typename ret<T>::type& operator[](T str)
	{
		constexpr std::size_t s = Get<0, T, JSONelem...>::i;
		return static_cast<typename ret<T>::type&>(std::get<s>(values));
	}
	template<class T>
	constexpr const typename ret<T>::type& operator[](T str) const
	{
		constexpr std::size_t s = Get<0, T, JSONelem...>::i;
		return static_cast<const typename ret<T>::type&>(std::get<s>(values));
	}
};



template<class T, T... chars>
Str<T, chars...> operator""_GCT()
{
	return {};
}




using name = JSONArray<decltype("name"_GCT), double, 2>;
using tree = JSONBranch<decltype("tree"_GCT), JSONLeaf<decltype("example1"), int>, JSONLeaf<decltype("example2"), int> >;
using password = JSONArray<decltype("password"_GCT), double, 2>;



int main()
{
	JSONBranch<decltype("root"_GCT), name, tree, password > a;	
	const decltype(a)& b = a;
	const double (&arr)[2] = b["name"_GCT];
	a["name"_GCT][1] = 32.2;
	//std::cout << decltype(a)::size << std::endl;
	
	//using Key = typename decltype(a)::Test<0,0>::value::type::StrKey;
	//char buffer[Key::size];
	//Key::get(buffer);

	//std::cout << buffer << std::endl;

	typename decltype(a)::Test<3, 0, false>::contain_type c;
	c.get;
}
