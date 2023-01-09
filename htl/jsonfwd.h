/**
 * @file htl/jsonfwd.h
 *
 * Forward declarations for JSON library
 */

#ifndef HTL_JSONFWD_H_
#define HTL_JSONFWD_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <memory_resource>

/**
 * JSON namespace
 */
namespace htl::json {

using Null = std::nullptr_t;
using Bool = bool;
using Int = std::int64_t;
using Float = double;

enum class Type : unsigned char {
    Null,
    Bool,
    Int,
    Float,
    String,
    Array,
    Object,
};

template <class Alloc>
class BasicDocument;

template <class Alloc>
class BasicString;

template <class Alloc>
class BasicArray;

template <class Alloc>
class BasicObject;

template <class Alloc>
class BasicParser;

template <class Alloc>
class BasicSerializer;

using Document = BasicDocument<std::allocator<std::byte>>;
using String = BasicString<std::allocator<std::byte>>;
using Array = BasicArray<std::allocator<std::byte>>;
using Object = BasicObject<std::allocator<std::byte>>;
using Parser = BasicParser<std::allocator<std::byte>>;
using Serializer = BasicSerializer<std::allocator<std::byte>>;

namespace pmr {

using Document = BasicDocument<std::pmr::polymorphic_allocator<std::byte>>;
using String = BasicString<std::pmr::polymorphic_allocator<std::byte>>;
using Array = BasicArray<std::pmr::polymorphic_allocator<std::byte>>;
using Object = BasicObject<std::pmr::polymorphic_allocator<std::byte>>;
using Parser = BasicParser<std::pmr::polymorphic_allocator<std::byte>>;
using Serializer = BasicSerializer<std::pmr::polymorphic_allocator<std::byte>>;

} // namespace pmr

enum class ParseErrorCode {
    None,
    UnexpectedToken,
    InvalidEscape,
    InvalidEncoding,
    MaxDepth,
    NumberOutOfRange,
    DuplicateKey,
};

struct ParseOptions {
    std::size_t max_depth = std::numeric_limits<std::size_t>::max();
    bool replace_invalid_code_points = true;
    bool accept_invalid_code_points = true;
    bool accept_trailing_commas = false;
    bool accept_comments = false;
    bool accept_duplicate_keys = false;
};

struct ParseError;

template <class I, class T>
struct ParseResult;

struct SerializeOptions {
    std::size_t indent_size = 0;
};

} // namespace htl::json

#endif
