#include "ContainerOrNumber.hpp"

ContainerOrNumber::ContainerOrNumber(const std::size_t num) noexcept: typeOfValue_(Tag::Number)
{
    value_.number = num;
}
ContainerOrNumber::ContainerOrNumber() noexcept(noexcept(ContainerType())): typeOfValue_(Tag::Container)
{
    new(std::addressof(value_.container)) ContainerType();
}
ContainerOrNumber::ContainerOrNumber(const ContainerOrNumber& item): typeOfValue_(item.typeOfValue_)
{
    CopyNode(item);
}

ContainerOrNumber::ContainerOrNumber(ContainerOrNumber&& item) noexcept: typeOfValue_(item.typeOfValue_)
{
    MoveNode(std::move(item));
}

ContainerOrNumber& ContainerOrNumber::operator=(const ContainerOrNumber& item)
{
    this->~ContainerOrNumber();
    new(this) ContainerOrNumber(item);
    return *this;
}
ContainerOrNumber& ContainerOrNumber::operator=(ContainerOrNumber&& item)
{
    this->~ContainerOrNumber();
    new(this) ContainerOrNumber(std::move(item));
    return *this;
}

ContainerOrNumber::~ContainerOrNumber()
{
    if(isContainer())
    {
        unsafeCastToContainer().~ContainerType();
    }
}
bool ContainerOrNumber::isContainer() noexcept
{
    return Tag::Container == typeOfValue_;
}

typename ContainerOrNumber::ContainerType& ContainerOrNumber::unsafeCastToContainer() noexcept
{
    return *static_cast<ContainerType*>(
        static_cast<void*>(
            std::addressof(value_.container)
        )
    );
}
const typename ContainerOrNumber::ContainerType& ContainerOrNumber::unsafeCastToContainer() const noexcept
{
    return *static_cast<const ContainerType*>(
        static_cast<const void*>(
            std::addressof(value_.container)
        )
    );
}
std::size_t& ContainerOrNumber::unsafeCastToNumber() noexcept
{
    return value_.number;
}

ContainerOrNumber& ContainerOrNumber::operator[](const std::string& key)
{
    return unsafeCastToContainer()[key];
}

const std::size_t& ContainerOrNumber::unsafeCastToNumber() const noexcept
{
    return value_.number;
}

const ContainerOrNumber& ContainerOrNumber::operator[](const std::string& key) const
{
    auto it = unsafeCastToContainer().find(key);
    return it->second;
}
void ContainerOrNumber::insert(const std::string& key, const ContainerOrNumber& value)
{
    unsafeCastToContainer()[key] = value;
}

void ContainerOrNumber::CopyNode(const ContainerOrNumber& node)
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
void ContainerOrNumber::MoveNode(ContainerOrNumber&& node) noexcept
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
