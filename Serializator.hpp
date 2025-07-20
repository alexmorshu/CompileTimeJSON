#pragma once
#include "CompileTimeJson.hpp"
template<class Iterator, class JsonType,  typename = void>
struct GetJSON;



template<class Iterator, class JsonType>
struct GetJSON<Iterator, JsonType, typename std::enable_if< !(ValueExists< typename Iterator::value>::type::value) >::type>
{

	static void get(std::string& container, std::size_t index, std::string& ctx, const JsonType& v)
	{
		using Branch = typename Iterator::value;
		char name [Branch::StrKey::size+2];
		name[0] = '"';
		name[Branch::StrKey::size+1] = '"';
		Branch::StrKey::get(name+1);
		
		if(index != 0)
		{
			container.push_back(',');
		}
		container.append(name);
		container.push_back(':');
		container.push_back('{');
		GetJSON<typename Branch::Iterator, Branch>::get(container, 0, ctx, v.getRaw(typename Branch::StrKey()));
		GetJSON<typename Iterator::next, JsonType>::get(container, index + 1, ctx, v);
	}
};

template<class Iterator, class JsonType>
struct GetJSON<Iterator, JsonType, typename std::enable_if<
	(ValueExists<typename Iterator::value>::type::value)>::type>
{
	static void get(std::string& container, std::size_t index, std::string& ctx, const JsonType& v)
	{
		using Element = typename Iterator::value;
		char name [Element::StrKey::size + 2];
		name[0] = '"';
		name[Element::StrKey::size+1] = '"';
		Element::StrKey::get(name + 1);
		if(index != 0)
		{
			container.push_back(',');
		}
		container.append(name);
		container.push_back(':');

		using converter = Converter<typename std::decay<decltype(v.getRaw(typename Element::StrKey()))>::type>;
		converter c;
		ctx.clear();
		c.ToString(ctx, v.getRaw(typename Element::StrKey()));
		container.append(ctx);
		index+=1;
		GetJSON<typename Iterator::next, JsonType>::get(container, index + 1, ctx, v);
	}
};

template<class JsonType>
struct GetJSON<Stop, JsonType>
{
	static void get(std::string& container, std::size_t index, std::string& ctx, const JsonType& v)
	{
		container.push_back('}');
	}
};

template<class T>
std::string serialize(const T& v)
{
	std::string result;
	std::string ctx;
	result.push_back('{');
	GetJSON<typename T::Iterator, T>::get(result, 0, ctx, v);
	return result;
}
