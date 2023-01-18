#ifndef HTL_DETAIL_JSON_H_
#define HTL_DETAIL_JSON_H_

#include <bit>
#include <charconv>
#include <climits>
#include <cmath>
#include <concepts>
#include <cuchar>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <cxxabi.h>
#include <htl/ascii.h>
#include <htl/detail/encoding.h>
#include <htl/jsonfwd.h>

namespace htl::json::detail {

template <class I, class S>
inline auto make_common_iterator(auto it)
{
    if constexpr (std::same_as<I, S>) {
        return it;
    } else {
        return std::common_iterator<I, S>(std::move(it));
    }
}

// https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf#page=49
inline bool unicode_is_noncharacter(char32_t value) noexcept
{
    return (value >= 0xFDD0 && value <= 0xFDEF) ||
           (value <= 0x10FFFF &&
            ((value & 0xFFFF) == 0xFFFE || (value & 0xFFFF) == 0xFFFF));
}

// https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf#page=49
inline bool unicode_is_surrogate(char32_t value) noexcept
{
    return value >= 0xD800 && value <= 0xDFFF;
}

// https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf#page=49
inline bool unicode_is_high_surrogate(char32_t value) noexcept
{
    return value >= 0xD800 && value <= 0xDBFF;
}

// https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf#page=49
inline bool unicode_is_low_surrogate(char32_t value) noexcept
{
    return value >= 0xDC00 && value <= 0xDFFF;
}

// No check for invalid surrogates.
inline char32_t unicode_surrogate_code_point(
    char16_t high, char16_t low) noexcept
{
    return ((static_cast<char32_t>(high) - 0xD800) << 10) +
           (static_cast<char32_t>(low) - 0xDC00) + 0x10000;
}

// See code point bit table:
// https://en.wikipedia.org/wiki/UTF-8#Encoding
template <class I, class S>
inline bool read_utf8_char(I &&first, S &&last, char32_t &code_point)
{
    if (first == last) {
        return false;
    }

    char8_t b1 = *first;

    if ((b1 >> 7) == 0b0) {
        code_point = b1;
    } else if (++first == last) {
        return false;
    } else if ((b1 >> 5) == 0b110) {
        char8_t b2 = *first;

        if ((b2 >> 6) != 0b10) {
            return false;
        }

        code_point = ((static_cast<char32_t>(b1) & 0x1F) << 6) |
                     (static_cast<char32_t>(b2) & 0x3F);
    } else if ((b1 >> 4) == 0b1110) {
        char8_t b2 = *first;

        if ((b2 >> 6) != 0b10 || ++first == last) {
            return false;
        }

        char8_t b3 = *first;

        if ((b3 >> 6) != 0b10) {
            return false;
        }

        code_point = //
            ((static_cast<char32_t>(b1) & 0x0F) << 12) |
            ((static_cast<char32_t>(b2) & 0x3F) << 6) |
            (static_cast<char32_t>(b3) & 0x3F);
    } else if ((b1 >> 3) == 0b11110) {
        char8_t b2 = *first;

        if ((b2 >> 6) != 0b10 || ++first == last) {
            return false;
        }

        char8_t b3 = *first;

        if ((b3 >> 6) != 0b10 || ++first == last) {
            return false;
        }

        char8_t b4 = *first;

        if ((b4 >> 6) != 0b10) {
            return false;
        }

        code_point =
            ((static_cast<char32_t>(b1) & 0x07) << 18) |
            ((static_cast<char32_t>(b2) & 0x3F) << 12) |
            ((static_cast<char32_t>(b3) & 0x3F) << 6) |
            (static_cast<char32_t>(b4) & 0x3F);
    } else {
        return false;
    }

    ++first;
    return true;
}

template <class O>
inline void write_char8(O &&out, char8_t c)
{
    *out = c;
    ++out;
}

template <class O>
inline std::size_t write_utf8_char(O &&out, char32_t code_point)
{
    if (code_point > 0x10FFFF) {
        return 0;
    } else if (!(code_point >> 7)) {
        write_char8(out, code_point);
        return 1;
    } else if (!(code_point >> 11)) {
        write_char8(out, 0xC0 | (code_point >> 6));
        write_char8(out, 0x80 | (code_point & 0x3F));
        return 2;
    } else if (!(code_point >> 16)) {
        write_char8(out, 0xE0 | (code_point >> 12));
        write_char8(out, 0x80 | ((code_point >> 6) & 0x3F));
        write_char8(out, 0x80 | (code_point & 0x3F));
        return 3;
    } else {
        write_char8(out, 0xF0 | (code_point >> 18));
        write_char8(out, 0x80 | ((code_point >> 12) & 0x3F));
        write_char8(out, 0x80 | ((code_point >> 6) & 0x3F));
        write_char8(out, 0x80 | (code_point & 0x3F));
        return 4;
    }
}

union PrimitiveValue {
    PrimitiveValue() noexcept = default;

    PrimitiveValue(Bool value) noexcept : bool_value(value) {}

    PrimitiveValue(std::integral auto value) noexcept : int_value(value) {}

    PrimitiveValue(std::floating_point auto value) noexcept : float_value(value)
    {}

    Null null_value;
    Bool bool_value;
    Int int_value;
    Float float_value;
};

template <class Alloc>
class Primitive {
public:
    Primitive() noexcept(noexcept(Alloc())) : _alloc(), _value() {}

    explicit Primitive(const Alloc &alloc) noexcept : _alloc(alloc), _value() {}

    Primitive(std::nullptr_t, const Alloc &alloc) noexcept
        : _alloc(alloc), _value()
    {}

    Primitive(Bool value, const Alloc &alloc) noexcept
        : _alloc(alloc), _value(value)
    {}

    Primitive(std::integral auto value, const Alloc &alloc) noexcept
        : _alloc(alloc), _value(value)
    {}

    Primitive(std::floating_point auto value, const Alloc &alloc) noexcept
        : _alloc(alloc), _value(value)
    {}

    Primitive(const Primitive &other, const Alloc &alloc) noexcept
        : _alloc(alloc), _value(other._value)
    {}

    Primitive(const Primitive &other) noexcept = default;

    Primitive &operator=(const Primitive &other) noexcept
    {
        if constexpr (std::allocator_traits<Alloc>::
                          propagate_on_container_copy_assignment::value) {
            _alloc = other._alloc;
        }

        _value = other._value;
        return *this;
    }

    Primitive &operator=(Primitive &&other) noexcept
    {
        if constexpr (std::allocator_traits<Alloc>::
                          propagate_on_container_move_assignment::value) {
            _alloc = other._alloc;
        }

        _value = other._value;
        return *this;
    }

    Primitive &operator=(Null) noexcept
    {
        return *this;
    }

    Primitive &operator=(Bool value) noexcept
    {
        _value.bool_value = value;
        return *this;
    }

    Primitive &operator=(std::integral auto value) noexcept
    {
        _value.int_value = static_cast<Int>(value);
        return *this;
    }

    Primitive &operator=(std::floating_point auto value) noexcept
    {
        _value.float_value = static_cast<Float>(value);
        return *this;
    }

    auto get_allocator() const noexcept
    {
        return _alloc;
    }

    auto &get_bool() noexcept
    {
        return _value.bool_value;
    }

    auto &get_int() noexcept
    {
        return _value.int_value;
    }

    auto &get_float() noexcept
    {
        return _value.float_value;
    }

    auto &get_bool() const noexcept
    {
        return _value.bool_value;
    }

    auto &get_int() const noexcept
    {
        return _value.int_value;
    }

    auto &get_float() const noexcept
    {
        return _value.float_value;
    }

    void swap(Primitive &other) noexcept
    {
        if constexpr (std::allocator_traits<
                          Alloc>::propagate_on_container_swap::value) {
            std::ranges::swap(_alloc, other._alloc);
        }

        std::ranges::swap(_value, other._value);
    }

private:
    [[no_unique_address]] Alloc _alloc;
    PrimitiveValue _value;
};

struct StringViewHash : std::hash<std::string_view> {
    using is_transparent = void;
};

struct StringViewEqual : std::equal_to<std::string_view> {
    using is_transparent = void;
};

template <class I, class S, class Alloc>
struct ParseHandler {
    using Stack =
        std::vector<BasicDocument<Alloc> *,
                    typename std::allocator_traits<Alloc>::rebind_alloc<
                        BasicDocument<Alloc> *>>;

    I first;
    S last;
    Stack stack;
    int line;
    int column;
    ParseErrorCode code;
    ParseOptions opts;
    [[no_unique_address]] Alloc alloc;

    ParseHandler(I first, S last, const Alloc &parser_alloc,
                 const Alloc &value_alloc, const ParseOptions &opts)
        : first(std::move(first)), last(std::move(last)), stack(parser_alloc),
          line(0), column(0), code(), opts(opts), alloc(value_alloc)
    {}

    ParseResult<I, BasicDocument<Alloc>> parse()
    {
        BasicDocument<Alloc> value(alloc);

        start_document(value);

        while (!has_error() && stack.size()) {
            if (stack.size() > opts.max_depth) {
                set_max_depth();
                break;
            }

            continue_document(*stack.back());
        }

        return { std::move(first), std::move(value), { code, line, column } };
    }

    bool has_error()
    {
        return code != ParseErrorCode();
    }

    char peek()
    {
        return static_cast<char>(*first);
    }

    bool done()
    {
        return first == last;
    }

    void skip()
    {
        ++first;
        ++column;
    }

    char next()
    {
        char c = peek();
        skip();
        return c;
    }

    void newline()
    {
        ++line;
        column = 0;
    }

    void expect_next(std::string_view s)
    {
        for (auto c: s) {
            if (done() || c != next()) {
                set_unexpected_token();
            }
        }
    }

    void start_document(BasicDocument<Alloc> &dest)
    {
        if (consume_whitespace_and_comments()) {
            return;
        }

        switch (peek()) {
        case '{':
            skip();
            dest.emplace_object();
            stack.push_back(std::addressof(dest));
            break;
        case '[':
            skip();
            dest.emplace_array();
            stack.push_back(std::addressof(dest));
            break;
        case '"':
            read_string(dest.emplace_string());
            break;
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            read_number(dest);
            break;
        case 't':
            expect_next("true");
            dest = true;
            break;
        case 'f':
            expect_next("false");
            dest = false;
            break;
        case 'n':
            expect_next("null");
            dest = nullptr;
            break;
        default:
            set_unexpected_token();
            break;
        }
    }

    void continue_document(BasicDocument<Alloc> &dest)
    {
        if (dest.is_array()) {
            continue_array(dest.get_array());
        } else {
            continue_object(dest.get_object());
        }
    }

    void continue_array(BasicArray<Alloc> &dest)
    {
        if (consume_whitespace_and_comments()) {
            return;
        }

        switch (peek()) {
        case ']':
            skip();
            stack.pop_back();
            break;
        case ',':
            skip();
            if (consume_whitespace_and_comments()) {
            } else if (peek() == ']') {
                if (opts.accept_trailing_commas && dest.size()) {
                    stack.pop_back();
                } else {
                    set_unexpected_token();
                }
            } else if (dest.empty()) {
                set_unexpected_token();
            } else {
                start_document(dest.emplace_back());
            }
            break;

        default:
            if (dest.size()) {
                set_unexpected_token();
            } else {
                start_document(dest.emplace_back());
            }
            break;
        }
    }

    void continue_object(BasicObject<Alloc> &dest)
    {
        if (consume_whitespace_and_comments()) {
            return;
        }

        switch (peek()) {
        case '}':
            skip();
            stack.pop_back();
            break;

        case ',':
            skip();
            if (!dest.size()) {
                set_unexpected_token();
            } else if (consume_whitespace_and_comments()) {
            } else if (peek() == '}') {
                if (opts.accept_trailing_commas) {
                    skip();
                    stack.pop_back();
                } else {
                    set_unexpected_token();
                }
            } else {
                start_entry(dest);
            }
            break;

        case '"':
            start_entry(dest);
            break;

        default:
            set_unexpected_token();
            break;
        }
    }

    void start_entry(BasicObject<Alloc> &dest)
    {
        Alloc alloc(dest.get_allocator());
        BasicString<Alloc> key(alloc);

        read_string(key);
        if (has_error() || consume_whitespace_and_comments()) {
            return;
        } else if (next() != ':') {
            set_unexpected_token();
            return;
        }

        auto [pos, inserted] = dest.try_emplace(std::move(key), alloc);
        if (!opts.accept_duplicate_keys && !inserted) {
            set_duplicate_key();
            return;
        }

        start_document(pos->second);
    }

    void read_number(BasicDocument<Alloc> &dest)
    {
        using BufferAlloc =
            typename std::allocator_traits<Alloc>::rebind_alloc<char>;
        using BufferType =
            std::basic_string<char, std::char_traits<char>, BufferAlloc>;

        BufferType buf(BufferAlloc(stack.get_allocator()));
        bool is_int = true;

        if (!done() && peek() == '-') {
            buf.push_back(next());
        }

        if (done() || !ascii_isdigit(peek())) {
            set_unexpected_token();
            return;
        } else if (peek() == '0') {
            buf.push_back(next());
        } else {
            while (!done() && ascii_isdigit(peek())) {
                buf.push_back(next());
            }
        }

        if (!done() && peek() == '.') {
            is_int = false;
            buf.push_back(next());

            if (done() || !ascii_isdigit(peek())) {
                set_unexpected_token();
                return;
            }

            while (!done() && ascii_isdigit(peek())) {
                buf.push_back(next());
            }
        }

        if (!done() && (peek() == 'e' || peek() == 'E')) {
            is_int = false;
            buf.push_back(next());

            if (done()) {
                set_unexpected_token();
                return;
            } else if (peek() == '+' || peek() == '-') {
                buf.push_back(next());
            }

            if (done() || !ascii_isdigit(peek())) {
                set_unexpected_token();
                return;
            } else if (peek() == '0') {
                buf.push_back(next());
            } else {
                while (!done() && ascii_isdigit(peek())) {
                    buf.push_back(next());
                }
            }
        }

        const char *first = buf.data();
        const char *last = first + buf.size();
        std::from_chars_result result;

        if (is_int) {
            dest = 0;
            result = std::from_chars(first, last, dest.get_int());
        } else {
            dest = 0.0;
            result = std::from_chars(first, last, dest.get_float());
        }

        switch (result.ec) {
        case std::errc::invalid_argument:
            set_unexpected_token();
            return;
        case std::errc::result_out_of_range:
            set_number_out_of_range();
            return;
        }

        if (result.ptr != last) {
            set_unexpected_token();
        }
    }

    void read_string(BasicString<Alloc> &dest)
    {
        if (done() || peek() != '"') {
            set_unexpected_token();
            return;
        }

        skip();
        for (char32_t code_point; !has_error();) {
            if (done()) {
                set_unexpected_token();
                return;
            } else if (!read_utf8_char(first, last, code_point)) {
                set_invalid_encoding();
                return;
            }

            if (code_point == U'"') {
                return;
            } else if (code_point == U'\\') {
                if (done()) {
                    set_unexpected_token();
                } else {
                    while (read_escape(dest) && !has_error()) {
                    }
                }
            } else if (code_point < 0x20) {
                set_unexpected_token();
            } else {
                append_code_point(dest, code_point);
            }
        }
    }

    bool read_escape(BasicString<Alloc> &dest)
    {
        // std::cout << "read_escape(): " << std::to_address(first)
        //           << ", peek(): " << peek() << "\n";

        if (done()) {
            set_unexpected_token();
            return false;
        }

        switch (next()) {
        case '"':
            dest.push_back('"');
            break;
        case '\\':
            dest.push_back('\\');
            break;
        case '/':
            dest.push_back('/');
            break;
        case 'b':
            dest.push_back('\b');
            break;
        case 'f':
            dest.push_back('\f');
            break;
        case 'n':
            dest.push_back('\n');
            break;
        case 'r':
            dest.push_back('\r');
            break;
        case 't':
            dest.push_back('\t');
            break;
        case 'u':
            return read_unicode_escape(dest);
        default:
            // std::cout << "default\n";
            set_invalid_escape();
            break;
        }

        return false;
    }

    bool read_unicode_escape(BasicString<Alloc> &dest)
    {
        char32_t code_point = read_unicode_escape_hex();

        if (has_error()) {
            return false;
        } else if (unicode_is_high_surrogate(code_point)) {
            return read_low_surrogate(dest, code_point);
        } else {
            append_code_point(dest, code_point);
        }

        return false;
    }

    bool read_low_surrogate(BasicString<Alloc> &dest, char16_t high)
    {
        if (done()) {
            set_unexpected_token();
        } else if (peek() == '\\') {
            skip();
            if (done()) {
                set_unexpected_token();
            } else if (peek() == 'u') {
                skip();

                char16_t low = read_unicode_escape_hex();
                if (has_error()) {
                    return false;
                } else if (unicode_is_low_surrogate(low)) {
                    append_code_point(
                        dest, unicode_surrogate_code_point(high, low));
                } else {
                    append_code_point(dest, high);
                    if (!has_error()) {
                        append_code_point(dest, low);
                    }
                }
            } else {
                append_code_point(dest, high);
                return true;
            }
        } else {
            append_code_point(dest, high);
        }

        return false;
    }

    void append_code_point(BasicString<Alloc> &dest, char32_t code_point)
    {
        if (unicode_is_surrogate(code_point) ||
            unicode_is_noncharacter(code_point)) {
            if (!opts.accept_invalid_code_points) {
                set_invalid_escape();
                return;
            } else if (opts.replace_invalid_code_points) {
                code_point = 0xFFFD;
            }
        }

        write_utf8_char(std::back_inserter(dest), code_point);
    }

    char16_t read_unicode_escape_hex()
    {
        char16_t value{};

        for (int i = 3; i >= 0; --i) {
            if (done()) {
                set_unexpected_token();
                return {};
            }

            int v = htl::detail::hex_values[static_cast<unsigned char>(peek())];
            if (v < 0) {
                set_invalid_escape();
                return {};
            }

            value |= v << (4 * i);
            skip();
        }

        return value;
    }

    bool consume_whitespace_and_comments()
    {
        while (true) {
            if (done()) {
                set_unexpected_token();
                return true;
            }

            switch (peek()) {
            case '/':
                consume_comment();
                if (has_error()) {
                    return true;
                }
                break;

            case '\r':
                skip();
                if (!done() && peek() == '\n') {
                    skip();
                }

                newline();
                break;

            case '\n':
                newline();
                skip();
                break;

            case '\t':
            case ' ':
                skip();
                break;

            default:
                return false;
            }
        }
    }

    void consume_comment()
    {
        if (!opts.accept_comments || done() || peek() != '/') {
            set_unexpected_token();
            return;
        }

        skip();
        if (done() || (peek() != '/' && peek() != '*')) {
            set_unexpected_token();
            return;
        }

        bool single = peek() == '/';
        skip();

        while (!done()) {
            switch (peek()) {
            case '\r':
                skip();
                if (!done() && peek() == '\n') {
                    skip();
                }

                newline();
                if (single) {
                    return;
                }
                break;

            case '\n':
                skip();
                newline();
                if (single) {
                    return;
                }
                break;

            case '*':
                skip();
                if (!single && !done() && peek() == '/') {
                    return;
                }
                break;

            default:
                skip();
                break;
            }
        }
    }

    void set_unexpected_token()
    {
        code = ParseErrorCode::UnexpectedToken;
    }

    void set_invalid_escape()
    {
        code = ParseErrorCode::InvalidEscape;
    }

    void set_max_depth()
    {
        code = ParseErrorCode::MaxDepth;
    }

    void set_number_out_of_range()
    {
        code = ParseErrorCode::NumberOutOfRange;
    }

    void set_duplicate_key()
    {
        code = ParseErrorCode::DuplicateKey;
    }

    void set_invalid_encoding()
    {
        code = ParseErrorCode::InvalidEncoding;
    }
};

template <class Alloc>
struct SerializePosition {
    using DocumentType = BasicDocument<Alloc>;
    using ArrayType = BasicArray<Alloc>;
    using ObjectType = BasicObject<Alloc>;
    using ArrayIt = ArrayType::const_iterator;
    using ObjectIt = ObjectType::const_iterator;

    SerializePosition(const DocumentType &value, ArrayIt it)
        : value(std::addressof(value)), array_it(it)
    {}

    SerializePosition(const DocumentType &value, ObjectIt it)
        : value(std::addressof(value)), object_it(it)
    {}

    ~SerializePosition() = default;

    ~SerializePosition()
        requires(!std::is_trivially_destructible_v<ArrayIt> ||
                 !std::is_trivially_destructible_v<ObjectIt>)
    {
        if (is_array()) {
            std::destroy_at(std::addressof(array_it));
        } else {
            std::destroy_at(std::addressof(object_it));
        }
    }

    bool is_array()
    {
        return value->is_array();
    }

    bool is_object()
    {
        return value->is_object();
    }

    bool is_array_first()
    {
        return array_it == value->get_array().begin();
    }

    bool is_array_last()
    {
        return array_it == value->get_array().end();
    }

    auto &array_next()
    {
        auto &value = *array_it;
        ++array_it;
        return value;
    }

    bool is_object_first()
    {
        return object_it == value->get_object().begin();
    }

    bool is_object_last()
    {
        return object_it == value->get_object().end();
    }

    auto &object_next()
    {
        auto &value = *object_it;
        ++object_it;
        return value;
    }

    const DocumentType *value;
    union {
        ArrayIt array_it;
        ObjectIt object_it;
    };
};

template <class O, class Alloc>
struct SerializeHandler {
    using StackAlloc = typename std::allocator_traits<Alloc>::rebind_alloc<
        SerializePosition<Alloc>>;

    using Stack = std::vector<SerializePosition<Alloc>, StackAlloc>;

    O out;
    Stack stack;
    SerializeOptions opts;
    std::size_t indent_depth;

    SerializeHandler(O out, const Alloc &alloc, const SerializeOptions &opts)
        : out(out), stack(StackAlloc(alloc)), opts(opts), indent_depth(0)
    {}

    void write(char c)
    {
        *out = c;
        ++out;
    }

    void write(const char *str)
    {
        for (; *str; ++str) {
            write(*str);
        }
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
        requires std::convertible_to<std::iter_reference_t<I>, char>
    void write(I first, S last)
    {
        for (; first != last; ++first) {
            write(*first);
        }
    }

    void write(std::string_view value)
    {
        for (char c: value) {
            write(c);
        }
    }

    void serialize(Null)
    {
        write("null");
    }

    void serialize(Bool value)
    {
        write(value ? "true" : "false");
    }

    void serialize(std::integral auto value)
    {
        char buf[std::numeric_limits<decltype(value)>::digits10 + 3];
        auto res = std::to_chars(buf, std::end(buf), value);

        write(buf, res.ptr);
    }

    void serialize(std::floating_point auto value)
    {
        // TODO: Fix size of buffer.
        char buf[std::numeric_limits<decltype(value)>::max_digits10 + 10];
        auto res = std::to_chars(buf, std::end(buf), value);

        write(buf, res.ptr);
    }

    void serialize(const BasicString<Alloc> &value)
    {
        write('"');

        auto first = value.data();
        auto last = first + value.size();

        for (; first != last;) {
            char32_t code_point;

            if (!read_utf8_char(first, last, code_point)) {
                code_point = 0xFFFD;
            }

            if (!write_escaped_character(code_point)) {
                write_utf8_char(out, code_point);
            }
        }

        write('"');
    }

    bool write_escaped_character(char32_t code_point)
    {
        if (code_point < 0x20) {
            write("\\u00");
            write(htl::detail::hex_charset_lower[code_point >> 4]);
            write(htl::detail::hex_charset_lower[code_point & 15]);
            return true;
        }

        switch (code_point) {
        case '"':
            write("\\\"");
            break;
        case '\\':
            write("\\\\");
            break;
        case '\b':
            write("\\\b");
            break;
        case '\f':
            write("\\\f");
            break;
        case '\n':
            write("\\\n");
            break;
        case '\r':
            write("\\\r");
            break;
        case '\t':
            write("\\\t");
            break;
        default:
            return false;
        }

        return true;
    }

    void serialize(const BasicArray<Alloc> &value)
    {
        write('[');

        if (value.size()) {
            indent();

            auto first = value.begin();
            auto last = value.end();

            serialize(*first);

            for (; first != last; ++first) {
                write(',');
                serialize(*first);
            }

            dedent();
        }

        write(']');
    }

    void serialize(const BasicObject<Alloc> &value)
    {
        write('{');

        if (value.size()) {
            indent();

            auto first = value.begin();
            auto last = value.end();

            write_indent();
            serialize(first->first);
            write(':');
            serialize(first->second);

            for (; first != last; ++first) {
                write(',');
                write_indent();
                serialize(first->first);
                write(':');
                serialize(first->second);
            }

            dedent();
        }

        write('}');
    }

    void serialize(const BasicDocument<Alloc> &value)
    {
        start_document(value);

        while (stack.size()) {
            continue_document(stack.back());
        }
    }

    void continue_document(auto &pos)
    {
        if (pos.is_array()) {
            continue_array(pos);
        } else {
            continue_object(pos);
        }
    }

    void continue_array(auto &pos)
    {
        if (pos.is_array_last()) {
            dedent();
            write(']');
            stack.pop_back();
            return;
        } else if (!pos.is_array_first()) {
            write(',');
            newline();
        }

        write_indent();
        start_document(pos.array_next());
    }

    void continue_object(auto &pos)
    {
        if (pos.is_object_last()) {
            dedent();
            write('}');
            stack.pop_back();
            return;
        } else if (!pos.is_object_first()) {
            write(',');
            newline();
        }

        write_indent();
        start_entry(pos.object_next());
    }

    void start_entry(auto &entry)
    {
        write_indent();
        serialize(entry.first);
        write(':');
        start_document(entry.second);
    }

    void start_document(auto &value)
    {
        switch (value.type()) {
        case Type::Null:
            serialize(nullptr);
            break;
        case Type::Bool:
            serialize(value.get_bool());
            break;
        case Type::Int:
            serialize(value.get_int());
            break;
        case Type::Float:
            serialize(value.get_float());
            break;
        case Type::String:
            serialize(value.get_string());
            break;
        case Type::Array:
            if (value.get_array().size()) {
                write('[');
                indent();
                stack.emplace_back(value, value.get_array().begin());
            } else {
                write("[]");
            }
            break;
        case Type::Object:
            if (value.get_object().size()) {
                write('{');
                indent();
                stack.emplace_back(value, value.get_object().begin());
            } else {
                write("{}");
            }
            break;
        }
    }

    void write_indent()
    {
        for (auto size = indent_depth * opts.indent_size; size; --size) {
            write(' ');
        }
    }

    void indent()
    {
        ++indent_depth;
        newline();
    }

    void dedent()
    {
        newline();
        --indent_depth;
    }

    void newline()
    {
        if (opts.indent_size) {
            write('\n');
        }
    }
};

} // namespace htl::json::detail

#endif
