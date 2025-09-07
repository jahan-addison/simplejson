
#include <iostream>
#include <simplejson.h>

using json::JSON;

int main()
{
    JSON Int = JSON::Load(" 123 ");
    JSON Float = JSON::Load(" 123.234 ");
    JSON Str = JSON::Load("\"String\"");
    JSON EscStr = JSON::Load("\" \\\"Some\\/thing\\\" \"");
    JSON Arr = JSON::Load("[1,2, true, false,\"STRING\", 1.5]");
    JSON Obj = JSON::Load("{ \"Key\" : \"StringValue\","
                          "   \"Key2\" : true, "
                          "   \"Key3\" : 1234, "
                          "   \"Key4\" : null }");

    std::cout << Int << std::endl;
    std::cout << Float << std::endl;
    std::cout << Str << std::endl;
    std::cout << EscStr << std::endl;
    std::cout << Arr << std::endl;
    std::cout << Obj << std::endl;
}
