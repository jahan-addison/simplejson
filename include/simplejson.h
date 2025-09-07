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
#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
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

    ~JSON() = default;

    explicit JSON(initializer_list<JSON> list)
        : JSON()
    {
        SetType(Class::Object);
        for (auto const& i : list) {
            this->operator[](i.ToString()) = i;
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
    {
        Type = other.Type;
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
    struct internal
    {
        using data =
            std::variant<std::monostate, long, double, bool, std::string>;
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

        std::optional<JSON_Deque_PTR> List{ std::nullopt };
        std::optional<JSON_String_PTR> String{ std::nullopt };
        std::optional<JSON_Map_PTR> Map{ std::nullopt };

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
    class JSONWrapper
    {
        using Container_PTR = std::shared_ptr<Container>;
        std::optional<Container_PTR> object;

      public:
        explicit JSONWrapper(Container_PTR val)
            : object(val)
        {
        }
        explicit JSONWrapper() = default;

        constexpr inline Container_PTR get() const noexcept
        {
            return object.value();
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
    class JSONConstWrapper
    {
        using Container_PTR = std::shared_ptr<Container>;
        std::optional<Container_PTR> object;

      public:
        explicit JSONConstWrapper(Container_PTR const val)
            : object(val)
        {
        }
        explicit JSONConstWrapper()
            : object(std::nullopt)
        {
        }

        constexpr inline const Container_PTR get() const noexcept
        {
            return object.value();
        }

        constexpr inline JSON& operator[](int index) const
        {
            return object.value()->at(index);
        }

        constexpr typename Container::const_iterator begin() const noexcept
        {
            return object ? object.value()->begin()
                          : typename Container::const_iterator();
        }
        constexpr typename Container::const_iterator end() const noexcept
        {
            return object ? object.value()->end()
                          : typename Container::const_iterator();
        }
    };

    static JSON Make(Class type)
    {
        JSON ret;
        ret.SetType(type);
        return ret;
    }

    static JSON Load(const std::string&) noexcept;

    template<typename T>
    void append(T arg)
    {
        SetType(Class::Array);
        Internal.List.value()->emplace_back(arg);
    }

    template<typename T, typename... U>
    void append(T arg, U... args)
    {
        append(arg);
        append(args...);
    }

    template<typename T>
    typename enable_if<std::is_same<T, bool>::value, JSON&>::type operator=(T b)
    {
        SetType(Class::Boolean);
        Internal.data_ = b;
        return *this;
    }

    template<typename T>
    typename enable_if<is_integral<T>::value && !std::is_same<T, bool>::value,
                       JSON&>::type
    operator=(T i)
    {
        SetType(Class::Integral);
        std::get<long>(Internal.data_) = i;
        return *this;
    }

    template<typename T>
    typename enable_if<is_floating_point<T>::value, JSON&>::type operator=(T f)
    {
        SetType(Class::Floating);
        std::get<double>(Internal.data_) = f;
        return *this;
    }

    template<typename T>
    typename enable_if<is_convertible<T, std::string>::value, JSON&>::type
    operator=(T s)
    {
        SetType(Class::String);
        Internal.String = make_data_object<JSON_String>(s);
        return *this;
    }

    JSON& operator[](std::string const& key) noexcept
    {
        SetType(Class::Object);
        return Internal.Map.value()->operator[](key);
    }

    const JSON& operator[](std::string const& key) const noexcept
    {
        SetType(Class::Object);
        return Internal.Map.value()->operator[](key);
    }

    JSON& operator[](unsigned index) noexcept
    {
        SetType(Class::Array);
        if (index >= Internal.List.value()->size())
            Internal.List.value()->resize(index + 1);

        return Internal.List.value()->operator[](index);
    }

    constexpr inline JSON& at(const std::string& key)
    {
        return Internal.Map.value()->operator[](key);
    }

    const JSON& at(const std::string& key) const
    {
        SetType(Class::Object);
        return Internal.Map.value()->operator[](key);
    }

    constexpr inline JSON& at(unsigned index) { return operator[](index); }

    constexpr inline const JSON& at(unsigned index) const
    {
        return Internal.List.value()->operator[](index);
    }

    constexpr inline size_t length() const
    {
        if (Type == Class::Array)
            return Internal.List.value()->size();
        else
            return 0;
    }

    constexpr inline bool hasKey(const std::string& key) const
    {
        if (Type == Class::Object) {
            return Internal.Map.value()->contains(key);
        }

        return false;
    }

    std::vector<std::string> dumpKeys() const
    {
        auto data = *(Internal.Map.value());
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

    constexpr inline Class JSONType() const noexcept { return Type; }

    constexpr inline bool IsNull() const { return Type == Class::Null; }

    inline std::string ToString() const noexcept
    {
        bool b;
        return ToString(b);
    }
    inline std::string ToString(bool& ok) const noexcept
    {
        ok = (Type == Class::String);
        return ok ? json_escape(*(Internal.String.value())) : std::string("");
    }

    inline double ToFloat() const noexcept
    {
        bool b;
        return ToFloat(b);
    }
    inline double ToFloat(bool& ok) const noexcept
    {
        ok = (Type == Class::Floating);
        return ok ? std::get<double>(Internal.data_) : 0.0;
    }

    inline long ToInt() const noexcept
    {
        bool b;
        return ToInt(b);
    }

    inline long ToInt(bool& ok) const noexcept
    {
        ok = (Type == Class::Integral);
        return ok ? std::get<long>(Internal.data_) : 0;
    }

    inline bool ToBool() const noexcept
    {
        bool b;
        return ToBool(b);
    }
    inline bool ToBool(bool& ok) const noexcept
    {
        ok = (Type == Class::Boolean);
        return ok ? std::get<bool>(Internal.data_) : false;
    }

    inline JSONWrapper<JSON_Map> ObjectRange() noexcept
    {
        return JSONWrapper<JSON_Map>(Internal.Map.value());
    }

    inline JSONWrapper<JSON_Deque> ArrayRange() noexcept
    {
        return JSONWrapper<JSON_Deque>(Internal.List.value());
    }

    inline JSONConstWrapper<JSON_Map> ObjectRange() const noexcept
    {
        return JSONConstWrapper<JSON_Map>(Internal.Map.value());
    }

    inline JSONConstWrapper<JSON_Deque> ArrayRange() const noexcept
    {
        return JSONConstWrapper<JSON_Deque>(Internal.List.value());
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
    void SetType(Class type) const noexcept
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

inline JSON Array() noexcept
{
    return JSON::Make(JSON::Class::Array);
}

template<typename... T>
inline JSON Array(T... args) noexcept
{
    JSON arr = JSON::Make(JSON::Class::Array);
    arr.append(args...);
    return arr;
}

inline JSON Object() noexcept
{
    return JSON::Make(JSON::Class::Object);
}

namespace {

// json construction functions

JSON parse_next(std::string const&, size_t&) noexcept;

void consume_ws(std::string const& str, size_t& offset) noexcept
{
    while (isspace(str[offset]))
        ++offset;
}

JSON parse_object(std::string const& str, size_t& offset) noexcept
{
    JSON object = JSON::Make(JSON::Class::Object);

    ++offset;
    consume_ws(str, offset);
    if (str[offset] == '}') {
        ++offset;
        return object;
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
        object[Key.ToString()] = Value;

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

    return object;
}

JSON parse_array(std::string const& str, size_t& offset) noexcept
{
    JSON array = JSON::Make(JSON::Class::Array);
    unsigned index = 0;

    ++offset;
    consume_ws(str, offset);
    if (str[offset] == ']') {
        ++offset;
        return array;
    }

    while (true) {
        array[index++] = parse_next(str, offset);
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
            return JSON::Make(JSON::Class::Array);
        }
    }

    return array;
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
                            return JSON::Make(JSON::Class::String);
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
                return JSON::Make(JSON::Class::Null);
            } else
                break;
        }
        exp = std::stol(exp_str);
    } else if (!isspace(c) && c != ',' && c != ']' && c != '}') {
        std::cerr << "ERROR: Number: unexpected character '" << c << "'\n";
        return JSON::Make(JSON::Class::Null);
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
        return JSON::Make(JSON::Class::Null);
    }
    offset += (Bool.ToBool() ? 4 : 5);
    return Bool;
}

JSON parse_null(std::string const& str, size_t& offset)
{
    JSON Null;
    if (str.substr(offset, 4) != "null") {
        std::cerr << "ERROR: Null: Expected 'null', found '"
                  << str.substr(offset, 4) << "'\n";
        return JSON::Make(JSON::Class::Null);
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

/////////////////////
// Main API function
////////////////////

inline JSON JSON::Load(std::string const& str) noexcept
{
    size_t offset = 0;
    return parse_next(str, offset);
}

} // namespace json
