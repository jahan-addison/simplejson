
#include <iostream>
#include <simplejson.h>

using json::JSON;

int main()
{
    // Example of creating each type
    // You can also do JSON::make( JSON::Class )
    JSON null;
    JSON Bool(true);
    JSON Str("RawString");
    JSON Str2(std::string("C++String"));
    JSON Int(1);
    JSON Float(1.2);
    JSON Arr = json::array();
    JSON Obj = json::object();

    // Types can be overwritten by assigning
    // to the object again.
    Bool = false;
    Bool = "rtew";
    Bool = 1;
    Bool = 1.1;
    Bool = std::string("asd");

    // Append to Arrays, appending to a non-array
    // will turn the object into an array with the
    // first element being the value that's being
    // appended.
    Arr.append(1);
    Arr.append("test");
    Arr.append(false);

    // Arrays can be intialized with any elements and
    // they are turned into JSON objects. Variadic
    // Templates are pretty cool.
    JSON Arr2 = json::array(2, "Test", true);

    // Objects are accessed using operator[]( string ).
    // Will create new pairs on the fly, just as std::map
    // would.
    Obj["Key1"] = 1.0;
    Obj["Key2"] = "Value";

    JSON Obj2 = json::object();
    Obj2["Key3"] = 1;
    Obj2["Key4"] = Arr;
    Obj2["Key5"] = Arr2;

    // Nested Object
    Obj["Key6"] = Obj2;

    // Dump Obj to a string.
    std::cout << Obj << std::endl;
}
