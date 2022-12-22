#ifndef HTL_DETAIL_JSON_H_
#define HTL_DETAIL_JSON_H_

#include <charconv>
#include <climits>
#include <concepts>
#include <cuchar>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <cxxabi.h>
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

// See code point bit table:
// https://en.wikipedia.org/wiki/UTF-8#Encoding
template <class I, class S>
inline bool read_utf8_char(I &first, S &last, char32_t &code_point)
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

        code_point = (static_cast<char32_t>(b1) << 6) | (b2 & 0x3F);
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
            (static_cast<char32_t>(b1) << 12) |
            ((static_cast<char32_t>(b2) & 0x3F) << 6) | (b3 & 0x3F);
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
            (static_cast<char32_t>(b1) << 18) |
            ((static_cast<char32_t>(b2) & 0x3F) << 12) |
            ((static_cast<char32_t>(b3) & 0x3F) << 6) | (b4 & 0x3F);
    }

    ++first;
    return true;
}

template <class O>
inline void write_char8(O &out, char8_t c)
{
    *out = c;
    ++out;
}

template <class O>
inline std::size_t write_utf8_char(O &out, char32_t code_point)
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

    void assign(Null) noexcept {}

    void assign(Bool value) noexcept
    {
        _value.bool_value = value;
    }

    void assign(std::integral auto value) noexcept
    {
        _value.int_value = static_cast<Int>(value);
    }

    void assign(std::floating_point auto value) noexcept
    {
        _value.float_value = static_cast<Float>(value);
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
    BasicDocument<Alloc> value;

    ParseHandler(I first, S last, const Alloc &parser_alloc,
                 const Alloc &value_alloc, const ParseOptions &opts)
        : first(std::move(first)), last(std::move(last)), stack(parser_alloc),
          line(0), column(0), code(), opts(opts), value(value_alloc)
    {}

    ParseResult<I, BasicDocument<Alloc>> parse()
    {
        start_document(value);

        while (!has_error() && stack.size()) {
            if (stack.size() >= opts.max_depth) {
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

    void next()
    {
        ++first;
    }

    void start_document(BasicDocument<Alloc> &dest)
    {
        if (consume_whitespace_and_comments()) {
            return;
        }

        switch (peek()) {
        case '{':
            next();
            dest.emplace_object();
            stack.push_back(std::addressof(dest));
            break;
        case '[':
            next();
            dest.emplace_array();
            stack.push_back(std::addressof(dest));
            break;
        case '"':
            dest.emplace_string();
            read_string(dest);
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
            next();
            stack.pop_back();
            break;
        case ',':
            next();
            if (consume_whitespace_and_comments()) {
                return;
            } else if (peek() == ']') {
                if (opts.allow_trailing_commas) {
                    stack.pop_back();
                } else {
                    set_unexpected_token();
                }
                return;
            } else if (!dest.size()) {
                set_unexpected_token();
                return;
            }

        default:
            start_document(dest.emplace_back());
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
            next();
            stack.pop_back();
            break;

        case ',':
            next();
            if (consume_whitespace_and_comments()) {
                return;
            } else if (peek() == '}') {
                if (opts.allow_trailing_commas) {
                    stack.pop_back();
                } else {
                    set_unexpected_token();
                }
                return;
            } else if (!dest.size() || peek() != '"') {
                set_unexpected_token();
                return;
            }

        case '"':
            start_object_entry(dest);
            break;

        default:
            set_unexpected_token();
            return;
        }
    }

    void start_object_entry(BasicObject<Alloc> &dest)
    {
        Alloc alloc(dest.get_allocator());
        BasicString<Alloc> key(BasicString<Alloc>::allocator_type(alloc));

        read_string(start_object_entry());
        if (has_error() || consume_whitespace_and_comments()) {
            return;
        } else if (peek() != ':') {
            set_unexpected_token();
            return;
        }

        auto [pos, inserted] = dest.try_emplace(std::move(key), alloc);
        if (!opts.allow_duplicate_keys && !inserted) {
            set_duplicate_key();
            return;
        }

        start_document(pos->second);
    }

    void read_string(BasicString<Alloc> &dest)
    {
        if (done() || peek() != '"') {
            set_unexpected_token();
            return;
        }

        next();
        while (true) {
            if (done()) {
                set_unexpected_token();
                return;
            }

            char c = peek();
            unsigned char uc = c;
            next();

            if (c == '"') {
                return;
            } else if (c == '\\') {
                read_string_escape(dest);
            } else if (ascii_iscntrl(c)) {
                set_unexpected_token();
            } else {
                dest.push_back(c);
            }
        }
    }

    void read_string_escape(char c);

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
                next();
                if (!done() && peek() == '\n') {
                    next();
                }

                ++line;
                break;

            case '\n':
                ++line;
                next();
                break;

            case '\t':
            case ' ':
                next();
                break;

            default:
                return false;
            }
        }
    }

    void consume_comment()
    {
        if (!opts.allow_comments || done() || peek() != '/') {
            set_unexpected_token();
            return;
        }

        next();
        if (done() || (peek() != '/' && peek() != '*')) {
            set_unexpected_token();
            return;
        }

        bool single = peek() == '/';
        next();

        while (!done()) {
            switch (peek()) {
            case '\r':
                next();
                if (!done() && peek() == '\n') {
                    next();
                }

                ++line;
                if (single) {
                    return;
                }
                break;

            case '\n':
                next();
                ++line;
                if (single) {
                    return;
                }
                break;

            case '*':
                next();
                if (!single && !done() && peek() == '/') {
                    return;
                }
                break;

            default:
                next();
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
        if (code_point < 0x20 || code_point == 0x7F) {
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
