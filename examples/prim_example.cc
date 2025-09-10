
#include <ios>
#include <iostream>
#include <simplejson.h>

using json::JSON;

int main()
{
    JSON obj;

    obj = true;
    std::cout << "Value: " << std::boolalpha << obj.to_bool() << std::endl;

    obj = "Test String";
    std::cout << "Value: " << obj.to_string() << std::endl;

    obj = 2.2;
    std::cout << "Value: " << obj.to_float() << std::endl;

    obj = 3;
    std::cout << "Value: " << obj.to_int() << std::endl;
}
