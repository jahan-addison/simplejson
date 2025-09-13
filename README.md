# simplejson++

simplejson++ is a lightweight JSON library for exporting data in JSON format from C++. Due to its design, you're able to create and work with JSON objects right away, just as you would expect from a language such as JavaScript. simplejson++ is a single C++ Header file, "simplejson.h".

simplejson++ requires at least C++17, and has been tested with clang-18 and gcc-10 on macOS (arm) and Ubuntu.

This library was primarily created for [Credence](https://github.com/jahan-addison/credence/).

---

## Major Features

* Intuitive construction of JSON objects and JSON from strings or file path
* STL-container type conversion and range access helpers for data types
* [No memory leaks](https://github.com/jahan-addison/simplejson/actions/runs/17600602459/job/50001743753#step:8:661)
* Total header size is _25kb_
* No dependencies, and strong test suite
* Compiles with Address, Undefined fsanitizers; valgrind; and `-Wall -Wextra -Werror -Wpedantic`
* Uses `constexpr` and `const` where possible
* Easy library installation via `FetchContent` or copying the header
* No use of `new` and `delete`
  * Uses `shared_ptr` where necessary, with no dangling pointers

See the api and example below, more in `examples/` directory.

## Installation

Either copy the header file into your project, or use `FetchContent` in cmake such as below:

```cmake
include(FetchContent)

FetchContent_Declare(
    simplejson
    GIT_REPOSITORY https://github.com/jahan-addison/simplejson.git
    GIT_TAG v1.1.12
)

FetchContent_MakeAvailable(simplejson)

# ...

target_include_directories(${PROJECT_NAME} PUBLIC simplejson)


```

### Example

```C++
#include <simplejson.h>

int main() {
  json::JSON obj;
  // Create a new Array as a field of an Object.
  obj["array"] = json::array( true, "Two", 3, 4.0 );
  // Create a new Object as a field of another Object.
  obj["obj"] = json::object();
  // Assign to one of the inner object's fields
  obj["obj"]["inner"] = "Inside";

  // We don't need to specify the type of the JSON object:
  obj["new"]["some"]["deep"]["key"] = "Value";
  obj["array2"].append( false, "three" );

  // We can also parse a std::string into a JSON object:
  obj["parsed"] = json::JSON::load( "[ { \"Key\" : \"Value\" }, false ]" );

  std::cout << obj << std::endl;
}
```

## API

### Overview

```C++
namespace json {

    /// Create a new JSON Array.
    JSON array( [any_type [, ... ] ] );

    /// Create a new JSON Object.
    JSON object();

    /// JSON Class. This is the core class.
    class JSON {

        enum Class {
            Null,
            Object,
            Array,
            String,
            Boolean,
            Floating,
            Integral
        };

        /**
            Static Methods
         */

        /// Create a JSON object from a std::string.
        JSON load( string_type );

        /// Create a JSON object from a json file.
        JSON load_file( string_type );

        /// Create a JSON object with the specified json::Class type.
        JSON make( JSON::Class );

        /**
            Access and Iterators
        */

        /// Returns a std::map of a Class::Object
        /// Will return empty map for non-object objects
        std::map<std::string, JSON> to_map();

        /// Wraps the internal object representation to access iterators
        /// Will return empty range for non-object objects
        JSON_Wrapper object_range();

        /// Returns a std::deque of a Class::Array
        /// Will return empty deque for non-array objects
        std::deque<JSON> to_deque();

        /// Wraps the internal array representation to access iterators.
        /// Will return empty range for non-array objects
        JSON_Wrapper array_range();

        /// Convience method to determine if an object is Class::Null
        bool is_null();

        /// Convert to a std::string literal iff Type == Class::String
        std::string to_string();

        /// Convert to a floating literal iff Type == Class::Floating
        double to_float();

        /// Convert to an integral literal iff Type == Class::Integral
        long to_int();

        /// Convert to a boolean literal iff Type == Class::Boolean
        bool to_bool();

        /// Get the JSON::Class type for a JSON object
        JSON::Class JSON_type();

        /**
            Typed Constructors

            string_type:  [const] char *, [const] char[], std::string, etc
            bool_type:    bool
            numeric_type: char, int, long, double, float, etc

         */
        JSON( string_type );
        JSON( bool_type );
        JSON( numeric_type );
        JSON( null_type );

        /**
            Copy/Move Constructors
         */
        JSON( const& JSON );
        JSON( JSON && );

        /**
            Operator Overloading

            Assigning to a JSON object changes the type on the fly.
            If you have a JSON object that represents an integer,
            and then you assign a boolean value to it, that object
            now represents a boolean.
         */

        /// Assign a boolean type to a JSON object
        JSON& operator=( bool_type );

        /// Compare equality of JSON objects by their internal data
        JSON& operator==( JSON const& other );

        /// Compare inequality of JSON objects by their internal data
        JSON& operator!=( JSON const& other );

        /// Assign a numeric type to a JSON object
        JSON& operator=( numeric_type );

        /// Assign a std::string type to a JSON object
        JSON& operator=( string_type );

        /// Assign a null type to a JSON object
        // JSON& operator=( null_type ); // TODO: Not Impld

        /// Standard copy/move assignment operators
        JSON& operator=( const& JSON );
        JSON& operator=( JSON && );

        /// Access the elements of a JSON Object.
        /// Accessing an invalid key will create a new entry with a Null type.
        JSON& operator[]( string_type key );

        /// Access the elements of a JSON Array.
        /// Accessing an out of bounds index will extend the Array.
        JSON& operator[]( unsigned index );

        /// Same as operator[]
        JSON& at( string_type | unsigned )

        /// const version of 'at'
        const JSON& at( string_type | unsigned ) const;

        /// Stream operator; calls dump()
        std::ostream& operator<<( std::ostream &, const& JSON );

        /**
            Utility Methods
         */

        /// Get the length of an array, or -1
        int length() const;

        /// Get the size of an Array or Object
        int size() const;

        /// Determine if an Object has a key
        bool has_key( string_type ) const;

        /// Useful for appending to an Array, can take any number of
        /// primitive types using variadic templates
        void append( any_type [, ... ] );

        /// Dumps the JSON object to a std::string format for storing.
        std::string dump( int depth = 0, std::string indent = "  " );

        /// Dumps the keys of a JSON object to a vector of strings
        std::vector<std::string> dump_keys();


    };
}
```
