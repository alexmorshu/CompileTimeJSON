#pragma once
#include "CompileTimeJson.hpp"
#include <iostream>
#include <cctype>
#include <stack>
#include <functional>

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





template<class JSONRoot>
void deserialize(const std::string& json, JSONRoot& container)
{
	static typename JSONRoot::MemberOfFunc members;
	static const ContainerOrNumber root = JSONRoot::getContainer();
	static bool isInit = false;
	if(!isInit)
	{
		JSONRoot::getArrayOfFunc(members);
		isInit = true;
	}

	std::size_t currentPosition = 0;
	std::string key;

	std::function<void(const ContainerOrNumber&)> reg = [&] (const ContainerOrNumber& cont)
	{
		if(cont.isContainer())
		{
			while(true)
			{
				currentPosition = json.find('"', currentPosition) + 1;
				const std::size_t last = json.find('"', currentPosition);
				key.assign(json.begin() + currentPosition, json.begin() + last);	
				currentPosition = json.find(':', last+1);
				do
					currentPosition+=1;
				while(std::isspace(json[currentPosition]));

				reg(cont[key]);				
				while(std::isspace(json[currentPosition]))
					currentPosition+=1;
				if(json[currentPosition] == '}')
					break;
				currentPosition+=1;
				
			}
		}
		else
		{	
			const std::size_t num = cont.unsafeCastToNumber();
			auto member = members[num];
			while(std::isspace(json[currentPosition]))
				currentPosition+=1;
			if(json[currentPosition] == '"')
			{	
				currentPosition += 1;
				const std::size_t last = json.find('"', currentPosition);
				key.assign(json.begin() + currentPosition, json.begin() + last);	
				(container.*member)(key.data(), key.size());
			}
			else if(json[currentPosition] == '[')
			{
				(container.*member)(json.c_str()+currentPosition, 0);
			}
			else
			{	
				const std::size_t old = currentPosition;
				while(!std::isspace(json[currentPosition]) && json[currentPosition] != ',' && json[currentPosition] != '}')
					currentPosition+=1;
	
				(container.*member)(json.c_str() + old, currentPosition-old);
			}

		}
	};
	reg(root);
}






