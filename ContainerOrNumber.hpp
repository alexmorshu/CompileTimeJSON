#pragma once
#include <unordered_map>
#include <string>

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
    ContainerOrNumber(const std::size_t num) noexcept;

    ContainerOrNumber() noexcept(noexcept(ContainerType()));

    ContainerOrNumber(const ContainerOrNumber& item);

    ContainerOrNumber(ContainerOrNumber&& item) noexcept;

    ContainerOrNumber& operator=(const ContainerOrNumber& item);

    ContainerOrNumber& operator=(ContainerOrNumber&& item);

    ~ContainerOrNumber();

    bool isContainer() noexcept;

    ContainerType& unsafeCastToContainer() noexcept;

    const ContainerType& unsafeCastToContainer() const noexcept;

    std::size_t& unsafeCastToNumber() noexcept;

    ContainerOrNumber& operator[](const std::string& key);

    const std::size_t& unsafeCastToNumber() const noexcept;

    const ContainerOrNumber& operator[](const std::string& key) const;

    void insert(const std::string& key, const ContainerOrNumber& value);

private:
    void CopyNode(const ContainerOrNumber& node);
    void MoveNode(ContainerOrNumber&& node) noexcept;

private:
    Node value_;
    Tag typeOfValue_;

};
