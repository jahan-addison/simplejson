
#include <ios>
#include <iostream>
#include <simplejson.h>

using json::JSON;

int main()
{
    JSON obj;

    obj = true;
    std::cout << "Value: " << std::boolalpha << obj.ToBool() << std::endl;

    obj = "Test String";
    std::cout << "Value: " << obj.ToString() << std::endl;

    obj = 2.2;
    std::cout << "Value: " << obj.ToFloat() << std::endl;

    obj = 3;
    std::cout << "Value: " << obj.ToInt() << std::endl;
}
