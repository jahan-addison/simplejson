
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#define CATCH2_TEST_BUILD 1

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <simplejson.h>
#include <string_view>

namespace fs = std::filesystem;

struct JSON_Fixture
{
    const std::string object_string = "{\n  \"a\" : {\n    \"b\" : \"c\"\n  "
                                      "},\n  \"b\": 1,\n  \"c\": false\n}";
    const std::string null_string = "null";
    const std::string array_string = "[\"a\",\"b\",\"c\",{\"d\": \"e\"}]";
    const std::string string_string = "Hello World";
    const double float_string = 0.00001;
    const bool bool_string = true;
    const long int_string = 5055559593;

    JSON_Fixture()
        : object_test(json::JSON::load(object_string))
        , array_test(json::JSON::load(array_string))
    {
    }

    json::JSON object_test;
    json::JSON null_test;
    json::JSON array_test;
    json::JSON string_test{ string_string };
    json::JSON float_test{ float_string };
    json::JSON bool_test{ bool_string };
    json::JSON int_test{ int_string };
};

inline std::string trim_string(std::string const& str)
{
    std::string new_string;
    std::copy_if(str.begin(),
                 str.end(),
                 std::back_inserter(new_string),
                 [](unsigned char c) { return !std::isspace(c); });

    return new_string;
}

TEST_CASE("Run all example cases without fail")
{
    fs::path current_directory = std::filesystem::current_path();

    const auto cases_path = fs::path(current_directory /= "test/cases");
    for (const auto& entry : fs::directory_iterator(cases_path)) {
        if (entry.is_regular_file()) {

            std::ifstream f(entry.path(), std::ios::in | std::ios::binary);
            const auto sz = fs::file_size(entry);
            std::string result(sz, '\0');
            f.read(result.data(), sz);

            REQUIRE_NOTHROW(json::JSON::load(result));

            f.close();
        }
    }
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::constructors")
{
    json::JSON test{ object_test };
    json::JSON test_2(false);
    json::JSON null;
    json::JSON test_3("hello");
    json::JSON test_4(static_cast<double>(0.000005));
    json::JSON test_5(555);

    REQUIRE(test.JSON_type() == json::JSON::Class::Object);
    REQUIRE(null.JSON_type() == json::JSON::Class::Null);
    REQUIRE(test_2.JSON_type() == json::JSON::Class::Boolean);
    REQUIRE(test_3.JSON_type() == json::JSON::Class::String);
    REQUIRE(test_4.JSON_type() == json::JSON::Class::Floating);
    REQUIRE(test_5.JSON_type() == json::JSON::Class::Integral);
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::operator==")
{
    json::JSON test{ object_test };
    json::JSON test2{ array_test };
    json::JSON test3 = json::JSON::load(object_string);
    json::JSON test4 = json::array();
    const std::string array_string_test = "[\"a\",\"b\",\"c\"]";
    test4[0] = json::JSON("a");
    test4[1] = json::JSON("b");
    test4[2] = json::JSON("c");
    REQUIRE(json::object == json::object);
    REQUIRE(json::JSON("test") == json::JSON("test"));
    REQUIRE(json::JSON(1) == json::JSON(1));
    REQUIRE(test4 == json::JSON::load(array_string_test));
    REQUIRE(test2 != object_test);
#ifdef __clang__
    REQUIRE(test2 == array_test);
    REQUIRE(test == object_test);
#endif
    REQUIRE(test3 == object_test);
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::array")
{
    json::JSON test = json::array();
    test[0] = json::JSON("a");
    test[1] = json::JSON("b");
    test[2] = json::JSON("c");
    test[3] = json::JSON::load("{\"d\" : \"e\"}");

    REQUIRE(trim_string(test) == trim_string(std::string{ array_test }));
    auto test_array = test.dump();
    REQUIRE(trim_string(test_array) == trim_string(array_string));

    test = json::array(true, false, 1);

    REQUIRE(test.JSON_type() == json::JSON::Class::Array);
    REQUIRE(trim_string(test) == "[true,false,1]");
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::object")
{
    json::JSON test = json::object();
    test["a"] = json::object();
    test["a"]["b"] = json::object();
    test["a"]["b"]["c"] = json::JSON("d");
    test["b"] = json::JSON(1);
    test["c"] = json::JSON(false);
    auto test_object = test.dump();
    auto expected = "{\n  \"a\" : {\n    \"b\" : {\n      \"c\" : \"d\"\n    "
                    "}\n  },\n  \"b\" : 1,\n  \"c\" : false\n}";
    REQUIRE(trim_string(test_object) == trim_string(expected));
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::load")
{
    json::JSON obj = json::JSON::load(object_string);

    REQUIRE(trim_string(obj) == trim_string(object_string));
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::load_file")
{
    json::JSON test_1 = json::JSON::load_file("./test/cases/test.json");
    json::JSON test_2 = json::JSON::load_file("./test/cases/test9.json");

    std::string expected =
        R"qaud({
  "product" : {
    "availability" : {
      "in_stock" : true,
      "last_restock" : "2025-08-20T10:30:00Z",
      "quantity" : 150
    },
    "category" : "Electronics",
    "name" : "Wireless Noise-Cancelling Headphones",
    "price" : 249.990000,
    "product_id" : "P001",
    "related_products" : ["P002", "P003"],
    "reviews" : [{
        "comment" : "Excellent sound quality and very comfortable for long\n  use.",
        "date" : "2025-09-01T14:15:00Z",
        "rating" : 5,
        "review_id" : "R001",
        "user_id" : "U101"
      }, {
        "comment" : "Noise cancellation works great, but charging cable is a\n  bit short.",
        "date" : "2025-08-28T09:00:00Z",
        "rating" : 4,
        "review_id" : "R002",
        "user_id" : "U102"
      }],
    "specifications" : {
      "battery_life_hours" : 30,
      "color" : ["Black", "Silver", "Midnight Blue"],
      "connectivity" : ["Bluetooth 5.2", "Auxiliary"],
      "features" : ["Active Noise Cancellation", "Transparency Mode", "VoiceAssistantIntegration"]
    }
  }
})qaud";
    auto expected_2 = "[1,2,3]";

    REQUIRE(std::string{ test_1 } == expected);
    REQUIRE(trim_string(test_2) == expected_2);
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::make")
{
    json::JSON obj = json::JSON::make(json::JSON::Class::Boolean);

    REQUIRE(obj.JSON_type() == json::JSON::Class::Boolean);
    REQUIRE(std::string{ obj } == "false");
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::to_map")
{
    auto test = object_test.to_map();
    REQUIRE(test.at("a").JSON_type() == json::JSON::Class::Object);
    std::string expected = R"qaud({"b":"c"})qaud";
    auto expected_2 = "[1,2,3]";
    REQUIRE(trim_string(std::string{ test.at("a") }) == expected);
    REQUIRE(test["b"] == json::JSON(1));
    REQUIRE(test["c"] == json::JSON(false));
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::object_range")
{
    auto expected = std::array<std::string, 3>{ "a", "b", "c" };
    size_t index = 0;
    for (auto const& test : object_test.object_range()) {
        REQUIRE(test.first == expected[index]);
        if (index == 0UL) {
            REQUIRE(test.second ==
                    json::JSON::load(" {\n    \"b\" : \"c\"\n  }"));
        }
        index++;
    }
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::to_deque")
{
    auto test = array_test.to_deque();
    std::string expected = R"qaud({"d":"e"})qaud";
    REQUIRE(test[0] == json::JSON{ "a" });
    REQUIRE(test[1] == json::JSON{ "b" });
    REQUIRE(test[2] == json::JSON{ "c" });
    REQUIRE(test[3] == json::JSON::load(expected));
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::array_range")
{
    auto expected = std::array<std::string, 4>{
        "\"a\"", "\"b\"", "\"c\"", R"qaud({"d":"e"})qaud"
    };
    size_t index = 0;
    for (auto const& test : array_test.array_range()) {
        REQUIRE(trim_string(std::string{ test }) == expected[index]);
        index++;
    }
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::is_null")
{
    REQUIRE(null_test.is_null() == true);
    REQUIRE(json::JSON{}.is_null() == true);
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::to_string")
{
    REQUIRE(string_test.to_string() == "Hello World");
    REQUIRE(int_test.to_string() == "");
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::to_float")
{
    REQUIRE(float_test.to_float() == float_string);
    REQUIRE(string_test.to_float() == 0.0);
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::to_int")
{
    REQUIRE(int_test.to_int() == int_string);
    REQUIRE(string_test.to_int() == 0);
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::to_bool")
{
    REQUIRE(bool_test.to_bool() == true);
    REQUIRE(string_test.to_bool() == false);
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::JSON_type")
{
    using namespace json;
    REQUIRE(object_test.JSON_type() == JSON::Class::Object);
    REQUIRE(array_test.JSON_type() == JSON::Class::Array);
    REQUIRE(bool_test.JSON_type() == JSON::Class::Boolean);
    REQUIRE(int_test.JSON_type() == JSON::Class::Integral);
    REQUIRE(float_test.JSON_type() == JSON::Class::Floating);
    REQUIRE(null_test.JSON_type() == JSON::Class::Null);
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::length")
{
    REQUIRE(array_test.length() == 4);
    REQUIRE(string_test.length() == -1);
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::size")
{
    REQUIRE(array_test.size() == 4);
    REQUIRE(object_test.size() == 3);
    REQUIRE(string_test.size() == -1);
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::has_key")
{
    REQUIRE(object_test.has_key("a") == true);
    REQUIRE(object_test.has_key("b") == true);
    REQUIRE(object_test.has_key("c") == true);
    REQUIRE(object_test.has_key("d") == false);
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::append")
{
    auto test = json::JSON::load(array_string);
    test.append("abc");
    test.append(123);
    REQUIRE(test.size() == 6);
    REQUIRE(test[test.size() - 1] == json::JSON{ 123 });
    REQUIRE(test[test.size() - 2] == json::JSON{ "abc" });
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::dump")
{
    REQUIRE(trim_string(object_test.dump()) == trim_string(object_string));
    REQUIRE(trim_string(array_test.dump()) == trim_string(array_string));
}

TEST_CASE_METHOD(JSON_Fixture, "JSON::dump_keys")
{
    auto expected = std::array<std::string, 3>{ "a", "b", "c" };
    size_t index = 0;
    for (auto const& key : object_test.dump_keys()) {
        REQUIRE(key == expected[index]);
        index++;
    }
}
