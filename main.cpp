#include <iostream>
#include <cstdint>
#include "CompileTimeString.hpp"
#include "ContainerOrNumber.hpp"
#include "CompileTimeJson.hpp"

using ID = JSONLeaf<decltype("ID"_GCT), std::uint64_t>;
using user = JSONBranch<decltype("user"_GCT), JSONLeaf<decltype("name"_GCT), const char*>, JSONLeaf<decltype("password"_GCT), const char*>>;
using root = JSONBranch<decltype("root"_GCT), ID, user>;

int main()
{
    root j;
    ContainerOrNumber container = j.getContainer();
    root::MemberOfFunc arr;
    root::getArrayOfFunc(arr);
    (j.*(arr[container["user"]["name"].unsafeCastToNumber()]))("124", 4);
    std::cout << j["user"_GCT]["name"_GCT] << std::endl;
}
