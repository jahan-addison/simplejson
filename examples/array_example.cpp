
#include <iostream>
#include <simplejson.h>

using json::JSON;

int main()
{
    JSON array;

    array[2] = "Test2";
    std::cout << array << std::endl;
    array[1] = "Test1";
    std::cout << array << std::endl;
    array[0] = "Test0";
    std::cout << array << std::endl;
    array[3] = "Test4";
    std::cout << array << std::endl;

    // Arrays can be nested:
    JSON Array2;

    Array2[2][0][1] = true;

    std::cout << Array2 << std::endl;
}
