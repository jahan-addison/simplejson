# simplejson++

simplejson++ is a lightweight JSON library for exporting data in JSON format from C++. Due to its design, you're able to create and work with JSON objects right away, just as you would expect from a language such as JavaScript. simplejson++ is a single C++ Header file, "simplejson.h".

simplejson++ requires at least C++20.

This library was primarily created for my [B Compiler, Credence](https://github.com/jahan-addison/credence/).

---

### Major Features

* Very intuitive and easy-to-use for constructing json from scratch, or load from disk
* [No memory leaks](https://github.com/jahan-addison/simplejson/actions/runs/17529039358/job/49783434847#step:5:661)
* Compiles with Address, Undefined sanitizers; valgrind; and `-Wall -Wextra -Werror -Wpedantic`
* Uses `constexpr` and `const` where possible
* cmake and enables cmake library installation via `FetchContent`
* No use of `new` and `delete`
  * Uses `shared_ptr` where necessary, with no dangling pointers
* Exposes `get` method on `ArrayRange` to get underlying iterator of an `JSON::Array`

### Main Methods

* `json::JSON::Load` static method to load large JSON from files or strings
* `json::JSON::dump` method to pretty-print json

* `json::JSON::dumpKeys` method to dump the keys of a `Class::Object`

See more examples below and in `examples/` directory.

## Installation

Either copy the header file into your project, or use `FetchContent` in cmake such as below:

```cmake
include(FetchContent)

FetchContent_Declare(
    simplejson
    GIT_REPOSITORY https://github.com/jahan-addison/simplejson.git
    GIT_TAG v1.1.2
)

FetchContent_MakeAvailable(simplejson)

# ...

target_include_directories(${PROJECT_NAME} PUBLIC simplejson)


```

# Example

```C++
#include <simplejson.h>

int main() {
  json::JSON obj;
  // Create a new Array as a field of an Object.
  obj["array"] = json::Array( true, "Two", 3, 4.0 );
  // Create a new Object as a field of another Object.
  obj["obj"] = json::Object();
  // Assign to one of the inner object's fields
  obj["obj"]["inner"] = "Inside";

  // We don't need to specify the type of the JSON object:
  obj["new"]["some"]["deep"]["key"] = "Value";
  obj["array2"].append( false, "three" );

  // We can also parse a string into a JSON object:
  obj["parsed"] = JSON::Load( "[ { \"Key\" : \"Value\" }, false ]" );

  std::cout << obj << std::endl;
}
```

# API

## Overview

```cpp
namespace json {

    /// Create a new JSON Array.
    JSON Array( [any_type [, ... ] ] );

    /// Create a new JSON Object.
    JSON Object();

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
            Typed Constructors

            string_type:  [const] char *, [const] char[], std::string, etc
            bool_type:    bool
            numeric_type: char, int, long, double, float, etc
            null_type:    nullptr_t

         */
        JSON( string_type );
        JSON( bool_type );
        JSON( numeric_type );
        JSON( null_type );

        /**
            Copy/Move Constructors
         */
        JSON( const JSON & );
        JSON( JSON && );

        /**
            Static Methods
         */

        /// Create a JSON object from a string.
        JSON Load( string_type );

        /// Create a JSON object with the specified json::Class type.
        JSON Make( JSON::Class );

        /**
            Operator Overloading

            Assigning to a JSON object changes the type on the fly.
            If you have a JSON object that represents an integer,
            and then you assign a boolean value to it, that object
            now represents a boolean.
         */

        /// Assign a boolean type to a JSON object
        JSON& operator=( bool_type );

        /// Assign a numeric type to a JSON object
        JSON& operator=( numeric_type );

        /// Assign a string type to a JSON object
        JSON& operator=( string_type );

        /// Assign a null type to a JSON object
        // JSON& operator=( null_type ); // TODO: Not Impld

        /// Standard copy/move assignment operators
        JSON& operator=( const JSON & );
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
        std::ostream& operator<<( std::ostream &, const JSON & );

        /**
            Utility Methods
         */

        /// Get the length of an array, or -1
        int length() const;

        /// Get the size of an Array or Object
        int size() const;

        /// Determine if an Object has a key
        bool hasKey( string_type ) const;

        /// Useful for appending to an Array, can take any number of
        /// primitive types using variadic templates
        void append( any_type [, ... ] );

        /// Dumps the JSON object to a string format for storing.
        void dump( int depth = 0, string indent = "  " );

        /// Get the JSON::Class type for a JSON object.
        JSON::Class JSONType();

        /// Convience method to determine if an object is Class::Null
        bool IsNull();

        /// Convert to a string literal iff Type == Class::String
        string ToString();
        string ToString( bool &OK );

        /// Convert to a floating literal iff Type == Class::Floating
        double ToFloat();
        double ToFloat( bool &OK );

        /// Convert to an integral literal iff Type == Class::Integral
        long ToInt();
        long ToInt( bool &OK );

        /// Convert to a boolean literal iff Type == Class::Boolean
        bool ToBool();
        bool ToBool( bool &OK );

        /**
            Iterating
        */

        /// Wraps the internal object representation to access iterators.
        /// Will return empty range for non-object objects.
        JSONWrapper ObjectRange();

        /// Wraps the internal array representation to access iterators.
        /// Will return empty range for non-array objects.
        JSONWrapper ArrayRange();
    }; // End json::JSON documentation
} // End json documentation
