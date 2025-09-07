
#include <filesystem>
#include <fstream>
#include <iostream>
#include <simplejson.h>
#include <string_view>

namespace fs = std::filesystem;

using json::JSON;

int main()
{
    fs::path current_directory = std::filesystem::current_path();
    try {
        for (const auto& entry :
             fs::directory_iterator(current_directory /= "../test/cases")) {
            if (entry.is_regular_file()) {
                std::cout << "Found file: " << entry.path().filename()
                          << std::endl;

                std::ifstream f(entry.path(), std::ios::in | std::ios::binary);
                const auto sz = fs::file_size(entry);
                std::string result(sz, '\0');
                f.read(result.data(), sz);

                JSON test_json = json::JSON::Load(result);

                std::cout << std::endl << test_json.dump() << std::endl;

                f.close();
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
