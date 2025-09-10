
#include <filesystem>
#include <iostream>
#include <simplejson.h>

using json::JSON;

int main()
{
    namespace fs = std::filesystem;
    JSON Int = JSON::load(" 123 ");
    JSON Float = JSON::load(" 123.234 ");
    JSON Str = JSON::load("\"String\"");
    JSON EscStr = JSON::load("\" \\\"Some\\/thing\\\" \"");
    JSON Arr = JSON::load("[1,2, true, false,\"STRING\", 1.5]");
    JSON Obj = JSON::load("{ \"Key\" : \"StringValue\","
                          "   \"Key2\" : true, "
                          "   \"Key3\" : 1234, "
                          "   \"Key4\" : null }");

    std::cout << Int << std::endl;
    std::cout << Float << std::endl;
    std::cout << Str << std::endl;
    std::cout << EscStr << std::endl;
    std::cout << Arr << std::endl;
    std::cout << Obj << std::endl;

    auto current_directory = fs::current_path();
    JSON json_file = json::JSON::load_file(
        (current_directory /= "examples/test.json").string());
    std::cout << "from file: " << json_file.dump() << std::endl;
    for (auto const& key : json_file["product"].dump_keys()) {
        std::cout << "keys from file: " << key << std::endl;
    }
}
