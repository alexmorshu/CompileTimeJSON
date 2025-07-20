#include <iostream>
#include <cstdint>
#include "CompileTimeString.hpp"
#include "ContainerOrNumber.hpp"
#include "CompileTimeJson.hpp"
#include "Serializator.hpp"
using ID = JSONLeaf<decltype("ID"_GCT), std::uint64_t>;
using user = JSONBranch
<
decltype("user"_GCT),
      JSONArray<
      		decltype("name"_GCT), JSONBranch
		<decltype("user"_GCT), JSONLeaf<decltype("u1"_GCT), std::uint64_t>, JSONLeaf<decltype("u2"_GCT), std::uint64_t
		>>,2
	>
	,JSONLeaf<decltype("password"_GCT), std::uint64_t>
>;
using root = JSONBranch<decltype("root"_GCT), ID, user>;

int main()
{
    root j;
    ContainerOrNumber container = j.getContainer();
    //root::MemberOfFunc arr;
    //root::getArrayOfFunc(arr);

    j["user"_GCT]["name"_GCT][0]["u1"_GCT] = 12;
    j["user"_GCT]["name"_GCT][1]["u2"_GCT]= 13;
    std::string json = serialize(j);
    std::cout << json << std::endl;
}
