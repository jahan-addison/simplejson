/***************
 * simplejson++
 ***************

 * Copyright (c) Jahan Addison
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **************************************************************************/

#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace json {

using std::enable_if;
using std::initializer_list;
using std::is_convertible;
using std::is_floating_point;
using std::is_integral;

class JSON;

namespace {

using JSON_Deque = std::deque<JSON>;
using JSON_String = std::string;
using JSON_Map = std::map<std::string, JSON>;
using JSON_Deque_PTR = std::shared_ptr<JSON_Deque>;
using JSON_String_PTR = std::shared_ptr<JSON_String>;
using JSON_Map_PTR = std::shared_ptr<JSON_Map>;
template<typename T>
concept Object_Variant =
    std::same_as<T, JSON_Deque> || std::same_as<T, JSON_String> ||
    std::same_as<T, JSON_Map>;
template<typename T>
concept Object_Variant_Pointer =
    std::same_as<T, JSON_Deque_PTR> || std::same_as<T, JSON_String_PTR> ||
    std::same_as<T, JSON_Map_PTR>;

constexpr std::string json_escape(const std::string& str)
{
    std::string output;
    for (unsigned i = 0; i < str.length(); ++i)
        switch (str[i]) {
            case '\"':
                output += "\\\"";
                break;
            case '\\':
                output += "\\\\";
                break;
            case '\b':
                output += "\\b";
                break;
            case '\f':
                output += "\\f";
                break;
            case '\n':
                output += "\\n";
                break;
            case '\r':
                output += "\\r";
                break;
            case '\t':
                output += "\\t";
                break;
            default:
                output += str[i];
                break;
        }
    return output;
}

template<Object_Variant Type>
inline std::shared_ptr<Type> make_data_object(Type const& obj)
{
    return std::make_shared<Type>(obj);
}

} // namespace

class JSON
{
  public:
    enum class Class
    {
        Null,
        Object,
        Array,
        String,
        Floating,
        Integral,
        Boolean
    };

  public:
    JSON()
        : Internal()
        , Type(Class::Null)
    {
    }
    explicit JSON(initializer_list<JSON> list)
        : JSON()
    {
        set_type(Class::Object);
        for (auto const& i : list) {
            this->operator[](i.to_string()) = i;
        }
    }
    explicit JSON(JSON&& other)
        : Internal(std::move(other.Internal))
        , Type(std::move(other.Type))
    {
    }
    JSON const& operator=(JSON&& other)
    {
        Internal = std::move(other.Internal);
        Type = std::move(other.Type);
        return *this;
    }
    JSON(const JSON& other)
        : Internal(other.Internal)
        , Type(other.Type)
    {
    }

    JSON& operator=(const JSON& other)
    {
        if (this == &other) {
            return *this;
        }
        Internal = other.Internal;
        Type = other.Type;
        return *this;
    }

    template<typename T>
    explicit JSON(
        T b,
        typename enable_if<std::is_same<T, bool>::value>::type* = nullptr)
        : Internal(b)
        , Type(Class::Boolean)
    {
    }

    template<typename T>
    explicit JSON(
        T i,
        typename enable_if<is_integral<T>::value &&
                           !std::is_same<T, bool>::value>::type* = nullptr)
        : Internal(static_cast<long>(i))
        , Type(Class::Integral)
    {
    }

    template<typename T>
    explicit JSON(
        T f,
        typename enable_if<is_floating_point<T>::value>::type* = nullptr)
        : Internal(static_cast<double>(f))
        , Type(Class::Floating)
    {
    }

    template<typename T>
    explicit JSON(
        T s,
        typename enable_if<is_convertible<T, std::string>::value>::type* =
            nullptr)
        : Internal(std::string(s))
        , Type(Class::String)
    {
    }

  private:
    // internal STL-container
    struct internal
    {
        using data = std::variant<std::monostate, long, double, bool>;
        using object = std::variant<JSON_Deque, JSON_String, JSON_Map>;

        internal() = default;

        explicit internal(long i)
            : data_{ i }
        {
        }
        explicit internal(double i)
            : data_{ i }
        {
        }
        explicit internal(bool i)
            : data_{ i }
        {
        }
        explicit internal(std::string const& i)
            : String(make_data_object<JSON_String>(i))
        {
        }
        template<Object_Variant_Pointer Type>
        explicit internal(Type&& i)
        {
            if constexpr (std::is_same_v<Type, JSON_Map>) {
                Map = std::move(i);
            } else if constexpr (std::is_same_v<Type, JSON_String>) {
                String = std::move(i);
            } else if constexpr (std::is_same_v<Type, JSON_Deque>) {
                List = std::move(i);
            }
        }
        std::optional<JSON_Deque_PTR> List;
        std::optional<JSON_String_PTR> String;
        std::optional<JSON_Map_PTR> Map;
        data data_ = std::monostate();
    };

  public:
    mutable internal Internal{};
    inline friend std::ostream& operator<<(std::ostream& os, const JSON& json)
    {
        os << json.dump();
        return os;
    }

    template<Object_Variant Type>
    friend std::optional<std::shared_ptr<Type>> make_data_object()
    {
        if constexpr (std::is_same_v<Type, JSON_Map>) {
            return std::make_shared<Type>(std::map<std::string, JSON>{});
        } else if constexpr (std::is_same_v<Type, JSON_String>) {
            return std::make_shared<Type>(std::string{});
        } else if constexpr (std::is_same_v<Type, JSON_Deque>) {
            return std::make_shared<JSON_Deque>(std::deque<JSON>{});
        } else {
            return std::make_shared<Type>(std::map<std::string, JSON>{});
        }
    }
    inline auto make_empty_map() const noexcept
    {
        return std::map<std::string, JSON>{};
    }
    inline auto make_empty_list() const noexcept { return std::deque<JSON>{}; }

    constexpr inline auto make_empty_string() const noexcept
    {
        return std::string{};
    }

    template<Object_Variant Type>
    constexpr Type get_safe_data_object(
        std::optional<std::shared_ptr<Type>> const& obj)
    {
        if constexpr (std::is_same_v<Type, JSON_Map>) {
            if (obj.has_value())
                return *(obj.value());
            else
                return make_empty_map();
        }
        if constexpr (std::is_same_v<Type, JSON_String>) {
            if (obj.has_value())
                return *(obj.value());
            else
                return make_empty_string();
        }
        if constexpr (std::is_same_v<Type, JSON_Deque>) {
            if (obj.has_value())
                return *(obj.value());
            else
                return make_empty_list();
        }
    }

    template<Object_Variant Container>
    class JSON_Wrapper
    {
        using Container_PTR = std::shared_ptr<Container>;
        std::optional<Container_PTR> object;

      public:
        explicit JSON_Wrapper(Container_PTR val)
            : object(val)
        {
        }

        explicit JSON_Wrapper()
            : object(std::nullopt)
        {
        }

        constexpr inline typename Container::iterator begin() noexcept
        {
            return object ? object.value()->begin()
                          : typename Container::iterator();
        }
        constexpr inline typename Container::iterator end() noexcept
        {
            return object ? object.value()->end()
                          : typename Container::iterator();
        }
        constexpr inline typename Container::const_iterator begin()
            const noexcept
        {
            return object ? object.value()->begin()
                          : typename Container::iterator();
        }
        constexpr inline typename Container::const_iterator end() const noexcept
        {
            return object ? object.value()->end()
                          : typename Container::iterator();
        }
    };

    template<Object_Variant Container>
    class JSON_Const_Wrapper
    {
        using Container_PTR = std::shared_ptr<Container>;
        std::optional<Container_PTR> object;

      public:
        explicit JSON_Const_Wrapper(Container_PTR const val)
            : object(val)
        {
        }
        explicit JSON_Const_Wrapper()
            : object(std::nullopt)
        {
        }

        constexpr inline JSON& operator[](int index) const
        {
            return object.value()->at(index);
        }
        constexpr inline typename Container::const_iterator begin()
            const noexcept
        {
            return object ? object.value()->begin()
                          : typename Container::const_iterator();
        }
        constexpr inline typename Container::const_iterator end() const noexcept
        {
            return object ? object.value()->end()
                          : typename Container::const_iterator();
        }
    };

    static inline JSON make(Class type)
    {
        JSON ret;
        ret.set_type(type);
        return ret;
    }

    static JSON load(std::string_view) noexcept;

    static JSON load_file(std::string_view);

    template<typename T>
    inline void append(T arg)
    {
        set_type(Class::Array);
        Internal.List.value()->emplace_back(arg);
    }

    template<typename T, typename... U>
    inline void append(T arg, U... args)
    {
        append(arg);
        append(args...);
    }

    template<typename T>
    typename enable_if<std::is_same<T, bool>::value, JSON&>::type operator=(T b)
    {
        set_type(Class::Boolean);
        Internal.data_ = b;
        return *this;
    }

    template<typename T>
    typename enable_if<is_integral<T>::value && !std::is_same<T, bool>::value,
                       JSON&>::type
    operator=(T i)
    {
        set_type(Class::Integral);
        std::get<long>(Internal.data_) = i;
        return *this;
    }

    template<typename T>
    typename enable_if<is_floating_point<T>::value, JSON&>::type operator=(T f)
    {
        set_type(Class::Floating);
        std::get<double>(Internal.data_) = f;
        return *this;
    }

    template<typename T>
    typename enable_if<is_convertible<T, std::string>::value, JSON&>::type
    operator=(T s)
    {
        set_type(Class::String);
        Internal.String = make_data_object<JSON_String>(s);
        return *this;
    }

    inline JSON& operator[](std::string const& key) noexcept
    {
        set_type(Class::Object);
        return Internal.Map.value()->operator[](key);
    }

    inline const JSON& operator[](std::string const& key) const noexcept
    {
        set_type(Class::Object);
        return Internal.Map.value()->operator[](key);
    }

    inline JSON& operator[](unsigned index) noexcept
    {
        set_type(Class::Array);
        if (index >= Internal.List.value()->size())
            Internal.List.value()->resize(index + 1);

        return Internal.List.value()->operator[](index);
    }

    inline JSON& at(const std::string& key)
    {
        return Internal.Map.value()->operator[](key);
    }

    inline const JSON& at(const std::string& key) const
    {
        set_type(Class::Object);
        return Internal.Map.value()->operator[](key);
    }

    inline JSON& at(unsigned index) { return operator[](index); }

    inline const JSON& at(unsigned index) const
    {
        return Internal.List.value()->operator[](index);
    }

    constexpr inline size_t length() const
    {
        if (Type == Class::Array)
            return Internal.List.value()->size();
        else
            return 0UL;
    }

    constexpr inline bool has_key(const std::string& key) const
    {
        if (Type == Class::Object) {
            return Internal.Map.value()->contains(key);
        }

        return false;
    }

    std::vector<std::string> dump_keys() const
    {
        auto data = *Internal.Map.value();
        std::vector<std::string> keys{};
        std::transform(data.begin(),
                       data.end(),
                       std::back_inserter(keys),
                       [](const auto& pair) { return pair.first; });
        return keys;
    }

    constexpr inline size_t size() const noexcept
    {
        if (Type == Class::Object)
            return Internal.Map.value()->size();
        else if (Type == Class::Array)
            return Internal.List.value()->size();
        else
            return -1UL;
    }

    constexpr inline Class JSON_type() const noexcept { return Type; }

    constexpr inline bool is_null() const { return Type == Class::Null; }

    constexpr inline JSON_String to_string() const noexcept
    {
        return Type == Class::String ? json_escape(*(Internal.String.value()))
                                     : std::string("");
    }

    inline JSON_Deque to_deque() const noexcept
    {
        return Type == Class::Array ? *Internal.List.value()
                                    : make_empty_list();
    }

    inline JSON_Map to_map() const noexcept
    {
        return Type == Class::Object ? *Internal.Map.value() : make_empty_map();
    }

    constexpr inline double to_float() const noexcept
    {
        return Type == Class::Floating ? std::get<double>(Internal.data_) : 0.0;
    }

    constexpr inline long to_int() const noexcept
    {
        return Type == Class::Integral ? std::get<long>(Internal.data_) : 0;
    }

    constexpr inline bool to_bool() const noexcept
    {
        return Type == Class::Boolean ? std::get<bool>(Internal.data_) : false;
    }

    inline JSON_Wrapper<JSON_Map> object_range() const noexcept
    {
        return JSON_Wrapper<JSON_Map>(Internal.Map.value());
    }

    inline JSON_Wrapper<JSON_Deque> array_range() const noexcept
    {
        return JSON_Wrapper<JSON_Deque>(Internal.List.value());
    }

    std::string dump(int depth = 1, std::string tab = "  ") const noexcept
    {
        std::string pad = "";
        for (int i = 0; i < depth; ++i, pad += tab)
            ;
        switch (Type) {
            case Class::Null:
                return "null";
            case Class::Object: {
                std::string s = "{\n";
                bool skip = true;
                for (auto& p : *(Internal.Map.value())) {
                    if (!skip)
                        s += ",\n";
                    s += (pad + "\"" + p.first +
                          "\" : " + p.second.dump(depth + 1, tab));
                    skip = false;
                }
                s += ("\n" + pad.erase(0, 2) + "}");
                return s;
            }
            case Class::Array: {
                std::string s = "[";
                bool skip = true;
                for (auto& p : *(Internal.List.value())) {
                    if (!skip)
                        s += ", ";
                    s += p.dump(depth + 1, tab);
                    skip = false;
                }
                s += "]";
                return s;
            }
            case Class::String:
                return "\"" + json_escape(*(Internal.String.value())) + "\"";
            case Class::Floating:
                return std::to_string(std::get<double>(Internal.data_));
            case Class::Integral:
                return std::to_string(std::get<long>(Internal.data_));
            case Class::Boolean:
                return std::get<bool>(Internal.data_) ? "true" : "false";
        }
        return "";
    }

    friend std::ostream& operator<<(std::ostream&, const JSON&);

  private:
    inline void set_type(Class type) const noexcept
    {
        if (type == Type)
            return;

        switch (type) {
            case Class::Null:
                break;
            case Class::Object:
                Internal.Map = make_data_object<JSON_Map>(make_empty_map());
                break;
            case Class::Array:
                Internal.List = make_data_object<JSON_Deque>(make_empty_list());
                break;
            case Class::String:
                Internal.String =
                    make_data_object<JSON_String>(make_empty_string());
                break;
            case Class::Floating:
                Internal.data_ = static_cast<double>(0.0);
                break;
            case Class::Integral:
                Internal.data_ = static_cast<int>(0);
                break;
            case Class::Boolean:
                Internal.data_ = static_cast<bool>(false);
                break;
        }

        Type = type;
    }

  private:
    mutable Class Type = Class::Null;
};

inline JSON array() noexcept
{
    return JSON::make(JSON::Class::Array);
}

template<typename... T>
inline JSON array(T... args) noexcept
{
    JSON arr = JSON::make(JSON::Class::Array);
    arr.append(args...);
    return arr;
}

inline JSON object() noexcept
{
    return JSON::make(JSON::Class::Object);
}

namespace {

// json construction functions

JSON parse_next(std::string const&, size_t&) noexcept;

inline void consume_ws(std::string const& str, size_t& offset) noexcept
{
    while (isspace(str[offset]))
        ++offset;
}

JSON parse_object(std::string const& str, size_t& offset) noexcept
{
    JSON obj = JSON::make(JSON::Class::Object);

    ++offset;
    consume_ws(str, offset);
    if (str[offset] == '}') {
        ++offset;
        return obj;
    }

    while (true) {
        JSON Key = parse_next(str, offset);
        consume_ws(str, offset);
        if (str[offset] != ':') {
            std::cerr << "Error: Object: Expected colon, found '" << str[offset]
                      << "'\n";
            break;
        }
        consume_ws(str, ++offset);
        JSON Value = parse_next(str, offset);
        obj[Key.to_string()] = Value;

        consume_ws(str, offset);
        if (str[offset] == ',') {
            ++offset;
            continue;
        } else if (str[offset] == '}') {
            ++offset;
            break;
        } else {
            std::cerr << "ERROR: object: Expected comma, found '" << str[offset]
                      << "'\n";
            break;
        }
    }

    return obj;
}

JSON parse_array(std::string const& str, size_t& offset) noexcept
{
    JSON arr = JSON::make(JSON::Class::Array);
    unsigned index = 0;

    ++offset;
    consume_ws(str, offset);
    if (str[offset] == ']') {
        ++offset;
        return arr;
    }

    while (true) {
        arr[index++] = parse_next(str, offset);
        consume_ws(str, offset);

        if (str[offset] == ',') {
            ++offset;
            continue;
        } else if (str[offset] == ']') {
            ++offset;
            break;
        } else {
            std::cerr << "ERROR: Array: Expected ',' or ']', found '"
                      << str[offset] << "'\n";
            return JSON::make(JSON::Class::Array);
        }
    }

    return arr;
}

JSON parse_string(std::string const& str, size_t& offset) noexcept
{
    JSON String;
    std::string val;
    for (char c = str[++offset]; c != '\"'; c = str[++offset]) {
        if (c == '\\') {
            switch (str[++offset]) {
                case '\"':
                    val += '\"';
                    break;
                case '\\':
                    val += '\\';
                    break;
                case '/':
                    val += '/';
                    break;
                case 'b':
                    val += '\b';
                    break;
                case 'f':
                    val += '\f';
                    break;
                case 'n':
                    val += '\n';
                    break;
                case 'r':
                    val += '\r';
                    break;
                case 't':
                    val += '\t';
                    break;
                case 'u': {
                    val += "\\u";
                    for (unsigned i = 1; i <= 4; ++i) {
                        c = str[offset + i];
                        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
                            (c >= 'A' && c <= 'F'))
                            val += c;
                        else {
                            std::cerr << "ERROR: String: Expected hex "
                                         "character in unicode escape, found '"
                                      << c << "'\n";
                            return JSON::make(JSON::Class::String);
                        }
                    }
                    offset += 4;
                } break;
                default:
                    val += '\\';
                    break;
            }
        } else
            val += c;
    }
    ++offset;
    String = val;
    return String;
}

JSON parse_number(std::string const& str, size_t& offset) noexcept
{
    JSON Number;
    std::string val, exp_str;
    char c;
    bool isDouble = false;
    long exp = 0;
    while (true) {
        c = str[offset++];
        if ((c == '-') || (c >= '0' && c <= '9'))
            val += c;
        else if (c == '.') {
            val += c;
            isDouble = true;
        } else
            break;
    }
    if (c == 'E' || c == 'e') {
        c = str[offset++];
        if (c == '-') {
            ++offset;
            exp_str += '-';
        }
        while (true) {
            c = str[offset++];
            if (c >= '0' && c <= '9')
                exp_str += c;
            else if (!isspace(c) && c != ',' && c != ']' && c != '}') {
                std::cerr
                    << "ERROR: Number: Expected a number for exponent, found '"
                    << c << "'\n";
                return JSON::make(JSON::Class::Null);
            } else
                break;
        }
        exp = std::stol(exp_str);
    } else if (!isspace(c) && c != ',' && c != ']' && c != '}') {
        std::cerr << "ERROR: Number: unexpected character '" << c << "'\n";
        return JSON::make(JSON::Class::Null);
    }
    --offset;

    if (isDouble)
        Number = std::stod(val) * std::pow(10, exp);
    else {
        if (!exp_str.empty())
            Number = std::stol(val) * std::pow(10, exp);
        else
            Number = std::stol(val);
    }
    return Number;
}
JSON parse_bool(std::string const& str, size_t& offset) noexcept
{
    JSON Bool;
    if (str.substr(offset, 4) == "true")
        Bool = true;
    else if (str.substr(offset, 5) == "false")
        Bool = false;
    else {
        std::cerr << "ERROR: Bool: Expected 'true' or 'false', found '"
                  << str.substr(offset, 5) << "'\n";
        return JSON::make(JSON::Class::Null);
    }
    offset += (Bool.to_bool() ? 4 : 5);
    return Bool;
}
JSON parse_null(std::string const& str, size_t& offset)
{
    JSON Null;
    if (str.substr(offset, 4) != "null") {
        std::cerr << "ERROR: Null: Expected 'null', found '"
                  << str.substr(offset, 4) << "'\n";
        return JSON::make(JSON::Class::Null);
    }
    offset += 4;
    return Null;
}
JSON parse_next(std::string const& str, size_t& offset) noexcept
{
    char value;
    consume_ws(str, offset);
    value = str[offset];
    switch (value) {
        case '[':
            return parse_array(str, offset);
        case '{':
            return parse_object(str, offset);
        case '\"':
            return parse_string(str, offset);
        case 't':
        case 'f':
            return parse_bool(str, offset);
        case 'n':
            return parse_null(str, offset);
        default:
            if ((value <= '9' && value >= '0') || value == '-')
                return parse_number(str, offset);
    }
    std::cerr << "ERROR: Parse: Unknown starting character '" << value << "'\n";
    return JSON();
}

} // namespace

///////////////////////
// Main API functions
//////////////////////

inline JSON JSON::load(std::string_view str) noexcept
{
    size_t offset = 0;
    return parse_next(str.data(), offset);
}

inline JSON JSON::load_file(std::string_view path)
{
    namespace fs = std::filesystem;
    std::ifstream f(path.data(), std::ios::in | std::ios::binary);
    const auto sz = fs::file_size(path);
    std::string result(sz, '\0');
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
    f.read(result.data(), sz);
#pragma GCC diagnostic pop

    return JSON::load(result);
}

} // namespace json
