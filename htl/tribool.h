#ifndef HTL_TRIBOOL_H_
#define HTL_TRIBOOL_H_

#include <compare>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

namespace htl {

class TriBool {
public:
    constexpr TriBool() noexcept = default;

    constexpr TriBool(std::nullptr_t) noexcept : TriBool() {}

    constexpr TriBool(bool state) noexcept
        : _state(static_cast<State>(state + 1))
    {}

    constexpr bool operator!() const noexcept
    {
        return _state == State::False;
    }

    constexpr explicit operator bool() const noexcept
    {
        return _state == State::True;
    }

    friend constexpr bool operator==(TriBool, TriBool) noexcept = default;

    friend constexpr std::strong_ordering operator<=>(TriBool, TriBool) //
        noexcept = default;

private:
    enum class State : std::uint8_t {
        Null,
        False,
        True,
    };

    State _state;
};

template <class CharT, class Traits>
constexpr std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &os, TriBool value)
{
    os << (value ? "true" : !value ? "false" : "null");
    return os;
}

} // namespace htl

#endif
