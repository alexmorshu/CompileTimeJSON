#pragma once
#include <type_traits>

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



template<class T>
struct Converter
{
    void ToString(std::string& str, const T& value) const noexcept
    {
    }


    void FromString(const char* c, std::size_t size , T& value) const noexcept
    {
    }
};

template<>
struct Converter<std::uint64_t>
{
    void ToString(std::string& str, const std::uint64_t& value)
    {
	    std::cout << value;
	    str = std::to_string(value);
    }


    void FromString(const char* c, std::size_t size , std::uint64_t& value)
    {
        int num = atoi(c);
        value = static_cast<std::uint64_t>(num);
    }
};
template<>
struct Converter<const char*>
{
    void ToString(std::string& str, const char* value)
    {
	    str.push_back('"');	
	    str.append(value);
	    str.push_back('"');	
    }


    void FromString(const char* c, std::size_t size , const char *(&value) )
    {
	   value = c;
    }
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



