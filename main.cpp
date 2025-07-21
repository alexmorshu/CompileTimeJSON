#include <iostream>
#include <cstdint>
#include "CompileTimeString.hpp"
#include "ContainerOrNumber.hpp"
#include "CompileTimeJson.hpp"
#include "Serializator.hpp"
using ID = JSONLeaf<decltype("ID"_GCT), std::uint64_t>;
using user = JSONBranch<decltype("user"_GCT), JSONLeaf<decltype("name"_GCT), std::uint64_t> , JSONLeaf<decltype("password"_GCT), std::uint64_t>
>;
using root = JSONBranch<decltype("root"_GCT), ID, user>;

int main()
{
    root j;
    ContainerOrNumber container = j.getContainer();
    root::MemberOfFunc arr;
    root::getArrayOfFunc(arr);

    std::string json = "{\"ID\":  43    ,   \"user\": { \"name\":12,\"password\":13}}";
    deserialize(json, j);

    std::cout << j["user"_GCT]["name"_GCT] << std::endl;
}
