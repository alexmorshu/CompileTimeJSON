#pragma once
#include <cstdint>
#include <cstddef>
#include <array>
#include <iostream>
#include <tuple>
#include <vector>
#include <algorithm>
#include <unordered_map>


//TODO: Add error support

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
	using type = std::false_type;
};

template<class T>
struct ValueExists<T, void_t<decltype(std::declval<T>().value)>>
{
	using type = std::true_type;
};


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


template<class T>
struct Converter
{
	void ToString(std::string& str, const T& value)
	{
	}


	void FromString(const char* c, std::size_t size , T& value)
	{
	}
};

template<>
struct Converter<std::uint64_t>
{
	void ToString(std::string& str, const std::uint64_t& value)
	{
	}


	void FromString(const char* c, std::size_t size , std::uint64_t& value)
	{
		int num = atoi(c);
		value = static_cast<std::uint64_t>(num);
	}
};


class ContainerOrNumber
{
	union Node;
	using ContainerType = std::unordered_map<std::string, ContainerOrNumber>;
	union Node
	{
		std::size_t number;
		alignas(ContainerType) char container[sizeof(ContainerType)];
	};
	enum class Tag
	{
		Number, Container
	};
public:
	ContainerOrNumber(const std::size_t num) noexcept: typeOfValue_(Tag::Number)
	{
		value_.number = num;
	}
	ContainerOrNumber() noexcept(noexcept(ContainerType())): typeOfValue_(Tag::Container)
	{
		new(std::addressof(value_.container)) ContainerType();
	}
	ContainerOrNumber(const ContainerOrNumber& item): typeOfValue_(item.typeOfValue_)
	{
		CopyNode(item);
	}

	ContainerOrNumber(ContainerOrNumber&& item) noexcept: typeOfValue_(item.typeOfValue_)
	{
		MoveNode(std::move(item));
	}

	ContainerOrNumber& operator=(const ContainerOrNumber& item)
	{
		this->~ContainerOrNumber();
		new(this) ContainerOrNumber(item);
		return *this;
	}
	ContainerOrNumber& operator=(ContainerOrNumber&& item)
	{
		this->~ContainerOrNumber();
		new(this) ContainerOrNumber(std::move(item));
		return *this;
	}

	~ContainerOrNumber()
	{
		if(isContainer())
		{
			unsafeCastToContainer().~ContainerType();
		}
	}
	bool isContainer() noexcept
	{
		return Tag::Container == typeOfValue_;
	}

	ContainerType& unsafeCastToContainer() noexcept
	{
		return *static_cast<ContainerType*>(
					static_cast<void*>(
						std::addressof(value_.container)
					)
				);
	}
	const ContainerType& unsafeCastToContainer() const noexcept
	{
		return *static_cast<const ContainerType*>(
					static_cast<const void*>(
						std::addressof(value_.container)
					)
				);
	}
	std::size_t& unsafeCastToNumber() noexcept
	{
		return value_.number;
	}

	ContainerOrNumber& operator[](const std::string& key)
	{
		return unsafeCastToContainer()[key];
	}	

	const std::size_t& unsafeCastToNumber() const noexcept
	{
		return value_.number;
	}

	const ContainerOrNumber& operator[](const std::string& key) const 
	{
		auto it = unsafeCastToContainer().find(key);
		return it->second;
	}	
	void insert(const std::string& key, const ContainerOrNumber& value)
	{
		unsafeCastToContainer()[key] = value;
	}
private: 
	void CopyNode(const ContainerOrNumber& node)
	{
		if(isContainer())
		{
			new(std::addressof(value_.container)) ContainerType(node.unsafeCastToContainer());
		}
		else
		{
			value_.number = node.value_.number; 
		}
	}
	void MoveNode(ContainerOrNumber&& node) noexcept
	{
		if(isContainer())
		{
			new(std::addressof(value_.container)) ContainerType(std::move(node.unsafeCastToContainer()));
		}
		else
		{
			value_.number = node.value_.number; 
		}
	}
private:
	Node value_;
	Tag typeOfValue_;

};


struct Stop
{};

template<std::size_t i, class... T>
struct Iterator;

template<std::size_t i,class f, class s, class... T>
struct Iterator<i, f, s, T...>
{
	static const std::size_t index = i;
	using value = f;
	using next = Iterator<i+1,s, T...>;
};

template<std::size_t i, class f>
struct Iterator<i,f>
{
	static const std::size_t index = i;
	using value = f;
	using next = Stop;
};



template<class Key, class ValueType>
struct JSONBaseElem
{
	static const std::size_t size = 1;
	using StrKey = Key;
	ValueType value;
	constexpr operator ValueType&() noexcept
	{
		return value;
	}
	constexpr operator const ValueType&() const noexcept
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

template<class BranchType, std::size_t size>
struct JSONBranchArray: JSONArray<typename BranchType::StrKey, BranchType, size>
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
	using Iterator = Iterator<0,JSONelem...>;
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
	{
		template<template <std::size_t...> class U>
		struct CopyInTemplate
		{
			using type = U<nums...>;
		};
	};

	

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
	struct GetSeq<n, n, index, false, f, JSONelems...>
	{
		using value = ContainerForGetSeg<n, index, f>;
	};

	template<std::size_t n, std::size_t current, std::size_t index, class f, class... JSONelems>
	struct GetSeq<n, current, index, false ,f, JSONelems...>
	{;
		using value = typename std::conditional<
				/*if*/ ((current + f::size) > n) ,
				/*then*/ ContainerForGetSeg<current, index, f>,
				/*else*/ typename GetSeq<n, current+f::size, index+1, ((current + f::size) > n) , JSONelems...>::value
					>::type;
	};


	//get NumsContainer for num
	template<std::size_t n, std::size_t current, class isElem, std::size_t... indexs>
	struct CalculateNumContainer
	{
		using value = typename GetSeq<n, current, 0, false ,JSONelem...>::value;
		using contain_type = NumsContainer<indexs..., value::i>;
	};

	template<std::size_t n, std::size_t current, std::size_t... indexs>
	struct CalculateNumContainer<n, current, typename ValueExists<typename GetSeq<n, current, 0, false ,JSONelem...>::value::type>::type, indexs...>
	{
		using value = typename GetSeq<n, current, 0, false ,JSONelem...>::value;
		using contain_type = typename value::type::template CalculateNumContainer<n, value::size, std::false_type ,indexs..., value::i>::contain_type;
	};


	//Get ref by indexes
	template<std::size_t... indexes>
	struct ForValue
	{
	};


	template<std::size_t f>
	struct ForValue<f>
	{
		using Ret = decltype(std::get<f>(std::declval<JSONBranch&>().values).value);	
		static constexpr Ret& getValue(JSONBranch& b) noexcept
		{
			return std::get<f>(b.values).value;
		}
		static constexpr const Ret& getValue(const JSONBranch& b) noexcept
		{
			return std::get<f>(b.values).value;
		}
	};
	template<std::size_t f, std::size_t s, std::size_t... indexes>
	struct ForValue<f, s, indexes...>
	{
		using nextBranch = typename std::remove_reference<decltype(std::get<f>(std::declval<JSONBranch>().values))>::type;
		using Ret = decltype(nextBranch::template ForValue<s, indexes...>::getValue(std::get<f>(std::declval<JSONBranch&>().values)));	
		using ConstRet = decltype(nextBranch::template ForValue<s, indexes...>::getValue(std::get<f>(std::declval<const JSONBranch&>().values)));	
		static constexpr Ret getValue(JSONBranch& b) noexcept
		{
			return nextBranch::template ForValue<s, indexes...>::getValue(std::get<f>(b.values));
		}
		static constexpr ConstRet getValue(const JSONBranch& b) noexcept
		{
			return nextBranch::template ForValue<s, indexes...>::getValue(std::get<f>(b.values));
		}
	};
	 
	//Get ForValue struct by only one arg
	template<std::size_t n>
	struct GetForValueStruct
	{
		using type = typename CalculateNumContainer<n,0, std::false_type>::contain_type::template CopyInTemplate<ForValue>::type;
	};
	


	template<std::size_t n>
	decltype(GetForValueStruct<n>::type::getValue(std::declval<JSONBranch&>())) getValue() noexcept
	{
		return GetForValueStruct<n>::type::getValue(*this); 
	}
	template<std::size_t n>
	decltype(GetForValueStruct<n>::type::getValue(std::declval<const JSONBranch&>())) getValue() const noexcept 
	{
		return GetForValueStruct<n>::type::getValue(*this); 
	}


	//Write in arg using only string
	template<std::size_t n>
	void Write(const char* str, std::size_t size)
	{
		using converter = Converter<typename std::decay<decltype(this->getValue<n>())>::type>; 
		converter c;
		c.FromString(str, size, this->getValue<n>());
	}

	using MemberOfFunc = void (JSONBranch::*([JSONBranch::size]))(const char*, std::size_t);
	struct Array
	{
		 static MemberOfFunc arr;
	};

	template<std::size_t i, std::size_t n>
	struct Func
	{
		static const void get(MemberOfFunc& arr) noexcept
		{
			arr[i] = &JSONBranch::Write<i>;
			Func<i+1, n>::get(arr);
		}
	};

	template<std::size_t n>
	struct Func<n, n>
	{	
		static const void get(MemberOfFunc&) noexcept
		{
			return;
		}
	};


	static void getArrayOfFunc(MemberOfFunc& a) noexcept
	{
		Func<0, JSONBranch::size>::get(a);
	};
	

	//Get num of type with key=Find
	template <std::size_t n, class Find, class... JSONelems>
	struct Get;

	template <std::size_t n, class Find, class f, class... JSONelems>
	struct Get<n, Find, f, JSONelems...>
	{
		static const std::size_t i = std::conditional<std::is_same<Find,typename f::StrKey>::value, std::integral_constant<std::size_t, n>, Get<n+1, Find, JSONelems...>>::type::value;
		static const std::size_t value = i;
	};


	//get type of json (branch/elem)
	template<class Find>
	struct typeJson
	{
		using type = decltype(std::get<Get<0, Find, JSONelem...>::i>(values));
	};


	//get type value, if it's a elem or branch
	template<class Find, class a = std::false_type>
	struct ret
	{
		using type = decltype(std::declval<typename typeJson<Find>::type>().value);
	};

	template<class Find>
	struct ret<Find, typename ValueExists< typename typeJson<Find>::type >::type >
	{
		using type = typename typeJson<Find>::type;
	};

	//get ref of value if it`s a elem with key str, or ref to branch
	template<class T>
	constexpr typename ret<T>::type& operator[](T str)
	{
		return static_cast<typename ret<T>::type&>(std::get<Get<0, T, JSONelem...>::i>(values));
	}
	template<class T>
	constexpr const typename ret<T>::type& operator[](T str) const
	{
		return static_cast<const typename ret<T>::type&>(std::get<Get<0, T, JSONelem...>::i>(values));
	}



	template<class Iterator, typename = void>
	struct GetContainer;



	template<class Iterator>
	struct GetContainer<Iterator, typename std::enable_if< !(ValueExists< typename Iterator::value>::type::value) >::type>
	{

		static void get(ContainerOrNumber& container, std::size_t& index)
		{
			using Branch = typename Iterator::value;
			char name [Branch::StrKey::size];
			Branch::StrKey::get(name);
			const std::string key(name, sizeof(name));
			ContainerOrNumber cont; 
			Branch::template GetContainer<typename Branch::Iterator>::get(cont, index);
			container.insert(key, cont);
			JSONBranch::template GetContainer<typename Iterator::next>::get(container, index);
		}
	};

	template<class Iterator>
	struct GetContainer<Iterator, typename std::enable_if<
		(ValueExists<typename Iterator::value>::type::value)>::type>
	{
		static void get(ContainerOrNumber& container, std::size_t& index)
		{
			using Element = typename Iterator::value;
			char name [Element::StrKey::size];
			Element::StrKey::get(name);
			const std::string key(name, sizeof(name));
			ContainerOrNumber cont(index);
			index+=1;
			container.insert(key, cont);
			JSONBranch::template GetContainer<typename Iterator::next>::get(container, index);
		}
	};

	template<>	
	struct GetContainer<Stop>
	{
		static void get(ContainerOrNumber& container, std::size_t& index)
		{
			return;
		}
	};

	
	static ContainerOrNumber getContainer() 
	{
		std::size_t index = 0;
		ContainerOrNumber result;
		GetContainer<Iterator>::get(result, index);
		return result;
	}


};



//GNU specific
template<class T, T... chars>
Str<T, chars...> operator""_GCT()
{
	return {};
}




using ID = JSONLeaf<decltype("ID"_GCT), std::uint64_t>;
using user = JSONBranch<decltype("user"_GCT), JSONLeaf<decltype("name"_GCT), const char*>, JSONLeaf<decltype("password"_GCT), const char*>>;
using root = JSONBranch<decltype("root"_GCT), ID, user>;


int main()
{
	root j;
	ContainerOrNumber container = j.getContainer();
	root::MemberOfFunc arr;
	root::getArrayOfFunc(arr);
	(j.*(arr[container["ID"].unsafeCastToNumber()]))("124", 4);
	std::cout << j["ID"_GCT] << std::endl;
}
