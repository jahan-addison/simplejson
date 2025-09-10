#include <iostream>
#include <simplejson.h>

int main()
{
    json::JSON obj;
    // Create a new Array as a field of an Object.
    obj["array"] = json::array(true, "Two", 3, 4.0);
    // Create a new Object as a field of another Object.
    obj["obj"] = json::object();
    // Assign to one of the inner object's fields
    obj["obj"]["inner"] = "Inside";

    // We don't need to specify the type of the JSON object:
    obj["new"]["some"]["deep"]["key"] = "Value";
    obj["array2"].append(false, "three");

    // We can also parse a std::string into a JSON object:
    obj["parsed"] = json::JSON::load("[ { \"Key\" : \"Value\" }, false ]");

    std::cout << obj << std::endl;
}
