/**
 * @file htl/json.h
 *
 * JSON library
 */

#ifndef HTL_JSON_H_
#define HTL_JSON_H_

#include <charconv>
#include <concepts>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include <htl/ascii.h>
#include <htl/concepts.h>
#include <htl/detail/json.h>
#include <htl/jsonfwd.h>
#include <htl/utility.h>

namespace htl::json {

template <class Alloc>
class BasicDocument {
public:
    using allocator_type = Alloc;

    BasicDocument() noexcept(noexcept(Alloc()))
        : _type(Type::Null), _primitive()
    {}

    explicit BasicDocument(const Alloc &alloc) noexcept
        : _type(Type::Null), _primitive(alloc)
    {}

    BasicDocument(Null, const Alloc &alloc = Alloc()) noexcept
        : _type(Type::Null), _primitive(alloc)
    {}

    BasicDocument(Bool value, const Alloc &alloc = Alloc()) noexcept
        : _type(Type::Bool), _primitive(value, alloc)
    {}

    BasicDocument(std::integral auto value,
                  const Alloc &alloc = Alloc()) noexcept
        : _type(Type::Int), _primitive(value, alloc)
    {}

    BasicDocument(std::floating_point auto value,
                  const Alloc &alloc = Alloc()) noexcept
        : _type(Type::Float), _primitive(value, alloc)
    {}

    BasicDocument(const char *value, const Alloc &alloc = Alloc())
        : _type(Type::String),
          _string(_new<BasicString<Alloc>>(alloc, value, StringAlloc(alloc)))
    {}

    BasicDocument(std::string_view value, const Alloc &alloc = Alloc())
        : _type(Type::String),
          _string(_new<BasicString<Alloc>>(alloc, value, StringAlloc(alloc)))
    {}

    BasicDocument(const BasicString<Alloc> &value)
        : _type(Type::String),
          _string(_new<BasicString<Alloc>>(value.get_allocator(), value))
    {}

    BasicDocument(const BasicArray<Alloc> &value)
        : _type(Type::Array),
          _array(_new<BasicArray<Alloc>>(value.get_allocator(), value))
    {}

    BasicDocument(const BasicObject<Alloc> &value)
        : _type(Type::Object),
          _object(_new<BasicObject<Alloc>>(value.get_allocator(), value))
    {}

    BasicDocument(BasicString<Alloc> &&value)
        : _type(Type::String),
          _string(
              _new<BasicString<Alloc>>(value.get_allocator(), std::move(value)))
    {}

    BasicDocument(BasicArray<Alloc> &&value)
        : _type(Type::Array),
          _array(
              _new<BasicArray<Alloc>>(value.get_allocator(), std::move(value)))
    {}

    BasicDocument(BasicObject<Alloc> &&value)
        : _type(Type::Object),
          _object(
              _new<BasicObject<Alloc>>(value.get_allocator(), std::move(value)))
    {}

    BasicDocument(const BasicString<Alloc> &value, const Alloc &alloc)
        : _type(Type::String),
          _string(_new<BasicString<Alloc>>(alloc, value, StringAlloc(alloc)))
    {}

    BasicDocument(const BasicArray<Alloc> &value, const Alloc &alloc)
        : _type(Type::Array),
          _array(_new<BasicArray<Alloc>>(alloc, value, ArrayAlloc(alloc)))
    {}

    BasicDocument(const BasicObject<Alloc> &value, const Alloc &alloc)
        : _type(Type::Object),
          _object(_new<BasicObject<Alloc>>(alloc, value, ObjectAlloc(alloc)))
    {}

    BasicDocument(BasicString<Alloc> &&value, const Alloc &alloc)
        : _type(Type::String),
          _string(_new<BasicString<Alloc>>(
              alloc, std::move(value), StringAlloc(alloc)))
    {}

    BasicDocument(BasicArray<Alloc> &&value, const Alloc &alloc)
        : _type(Type::Array),
          _array(_new<BasicArray<Alloc>>(
              alloc, std::move(value), ArrayAlloc(alloc)))
    {}

    BasicDocument(BasicObject<Alloc> &&value, const Alloc &alloc)
        : _type(Type::Object),
          _object(_new<BasicObject<Alloc>>(
              alloc, std::move(value), ObjectAlloc(alloc)))
    {}

    BasicDocument(const BasicDocument &other)
        : BasicDocument(
              other,
              std::allocator_traits<Alloc>::
                  select_on_container_copy_construction(other.get_allocator()))
    {}

    BasicDocument(const BasicDocument &other, const Alloc &alloc)
    {
        _copy_construct(other, alloc);
    }

    BasicDocument(BasicDocument &&other) noexcept
    {
        _move_construct(std::move(other));
    }

    BasicDocument(BasicDocument &&other, const Alloc &alloc) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value)
    {
        if (std::allocator_traits<Alloc>::is_always_equal::value ||
            other.get_allocator() == alloc) {
            _move_construct(std::move(other));
        } else {
            _copy_construct(other, alloc);
        }
    }

    ~BasicDocument() noexcept
    {
        _destroy();
    }

    BasicDocument &operator=(const BasicDocument &other)
    {
        if (this != std::addressof(other)) {
            _copy_assign(other);
        }

        return *this;
    }

    BasicDocument &operator=(BasicDocument &&other) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value ||
                 std::allocator_traits<
                     Alloc>::propagate_on_container_copy_assignment::value)
    {
        if (this != std::addressof(other)) {
            _move_assign(std::move(other));
        }

        return *this;
    }

    void assign(Null) noexcept
    {
        _assign_primitive(nullptr, Type::Null);
    }

    void assign(Bool value) noexcept
    {
        _assign_primitive(value, Type::Bool);
    }

    void assign(std::integral auto value) noexcept
    {
        _assign_primitive(value, Type::Int);
    }

    void assign(std::floating_point auto value) noexcept
    {
        _assign_primitive(value, Type::Float);
    }

    void assign(const char *value)
    {
        _assign_string(value);
    }

    void assign(std::string_view value)
    {
        _assign_string(value);
    }

    void assign(const BasicString<Alloc> &value)
    {
        _assign_string(value);
    }

    void assign(const BasicArray<Alloc> &value)
    {
        _assign_array(value);
    }

    void assign(const BasicObject<Alloc> &value)
    {
        _assign_object(value);
    }

    void assign(BasicString<Alloc> &&value)
    {
        _assign_string(std::move(value));
    }

    void assign(BasicArray<Alloc> &&value)
    {
        _assign_array(std::move(value));
    }

    void assign(BasicObject<Alloc> &&value)
    {
        _assign_object(std::move(value));
    }

    BasicString<Alloc> &emplace_string(auto &&...args)
    {
        auto alloc = get_allocator();
        auto prev = std::move(*this);
        try {
            _destroy_at_primitive();
            _construct_at_string(_new<BasicString<Alloc>>(
                alloc, std::forward<decltype(args)>(args)...,
                StringAlloc(alloc)));
            _type = Type::String;
        } catch (...) {
            _move_construct(std::move(prev));
            throw;
        }
        return *_string;
    }

    BasicArray<Alloc> &emplace_array(auto &&...args)
    {
        auto alloc = get_allocator();
        auto prev = std::move(*this);
        try {
            _destroy_at_primitive();
            _construct_at_array(_new<BasicArray<Alloc>>(
                alloc, std::forward<decltype(args)>(args)...,
                ArrayAlloc(alloc)));
            _type = Type::Array;
        } catch (...) {
            _move_construct(std::move(prev));
            throw;
        }
        return *_array;
    }

    BasicObject<Alloc> &emplace_object(auto &&...args)
    {
        auto alloc = get_allocator();
        auto prev = std::move(*this);
        try {
            _destroy_at_primitive();
            _construct_at_object(_new<BasicObject<Alloc>>(
                alloc, std::forward<decltype(args)>(args)...,
                ObjectAlloc(alloc)));
            _type = Type::Object;
        } catch (...) {
            _move_construct(std::move(prev));
            throw;
        }

        return *_object;
    }

    Alloc get_allocator() const noexcept
    {
        switch (_type) {
        case Type::Null:
        case Type::Bool:
        case Type::Int:
        case Type::Float:
            return _primitive.get_allocator();
        case Type::String:
            return _string->get_allocator();
        case Type::Array:
            return _array->get_allocator();
        case Type::Object:
            return _object->get_allocator();
        }

        unreachable();
    }

    Type type() const noexcept
    {
        return _type;
    }

    bool is_null() const noexcept
    {
        return type() == Type::Null;
    }

    bool is_bool() const noexcept
    {
        return type() == Type::Bool;
    }

    bool is_int() const noexcept
    {
        return type() == Type::Int;
    }

    bool is_float() const noexcept
    {
        return type() == Type::Float;
    }

    bool is_string() const noexcept
    {
        return type() == Type::String;
    }

    bool is_array() const noexcept
    {
        return type() == Type::Array;
    }

    bool is_object() const noexcept
    {
        return type() == Type::Object;
    }

    Bool &get_bool() &noexcept
    {
        return _primitive.get_bool();
    }

    Bool &&get_bool() &&noexcept
    {
        return std::move(_primitive.get_bool());
    }

    const Bool &get_bool() const &noexcept
    {
        return _primitive.get_bool();
    }

    const Bool &&get_bool() const &&noexcept
    {
        return std::move(_primitive.get_bool());
    }

    Int &get_int() &noexcept
    {
        return _primitive.get_int();
    }

    Int &&get_int() &&noexcept
    {
        return std::move(_primitive.get_int());
    }

    const Int &get_int() const &noexcept
    {
        return _primitive.get_int();
    }

    const Int &&get_int() const &&noexcept
    {
        return std::move(_primitive.get_int());
    }

    Float &get_float() &noexcept
    {
        return _primitive.get_float();
    }

    Float &&get_float() &&noexcept
    {
        return std::move(_primitive.get_float());
    }

    const Float &get_float() const &noexcept
    {
        return _primitive.get_float();
    }

    const Float &&get_float() const &&noexcept
    {
        return std::move(_primitive.get_float());
    }

    BasicString<Alloc> &get_string() &noexcept
    {
        return *_string;
    }

    BasicString<Alloc> &&get_string() &&noexcept
    {
        return std::move(*_string);
    }

    const BasicString<Alloc> &get_string() const &noexcept
    {
        return *_string;
    }

    const BasicString<Alloc> &&get_string() const &&noexcept
    {
        return *std::move(*_string);
    }

    BasicArray<Alloc> &get_array() &noexcept
    {
        return *_array;
    }

    BasicArray<Alloc> &&get_array() &&noexcept
    {
        return std::move(*_array);
    }

    const BasicArray<Alloc> &get_array() const &noexcept
    {
        return *_array;
    }

    const BasicArray<Alloc> &&get_array() const &&noexcept
    {
        return std::move(*_array);
    }

    BasicObject<Alloc> &get_object() &noexcept
    {
        return *_object;
    }

    BasicObject<Alloc> &&get_object() &&noexcept
    {
        return std::move(*_object);
    }

    const BasicObject<Alloc> &get_object() const &noexcept
    {
        return *_object;
    }

    const BasicObject<Alloc> &&get_object() const &&noexcept
    {
        return std::move(*_object);
    }

    void swap(BasicDocument &other) //
        noexcept(
            std::allocator_traits<Alloc>::is_always_equal::value ||
            std::allocator_traits<Alloc>::propagate_on_container_swap::value);

    friend void swap(BasicDocument &a, BasicDocument &b) //
        noexcept(noexcept(a.swap(b)))
    {
        a.swap(b);
    }

    friend bool operator==(const BasicDocument &a, const BasicDocument &b)
    {
        if (a.type() == b.type()) {
            switch (a.type()) {
            case Type::Null:
                return true;
            case Type::Bool:
                return a.get_bool() == b.get_bool();
            case Type::Int:
                return a.get_int() == b.get_int();
            case Type::Float:
                return a.get_float() == b.get_float();
            case Type::String:
                return a.get_string() == b.get_string();
            case Type::Array:
                return a.get_array() == b.get_array();
            case Type::Object:
                return a.get_object() == b.get_object();
            }
        }

        return false;
    }

private:
    using StringAlloc =
        typename std::allocator_traits<Alloc>::rebind_alloc<char>;

    using ArrayAlloc = typename std::allocator_traits<Alloc>::rebind_alloc<
        BasicDocument<Alloc>>;

    using ObjectAlloc = typename std::allocator_traits<Alloc>::rebind_alloc<
        std::pair<const BasicString<Alloc>, BasicDocument<Alloc>>>;

    template <class T>
    using AllocatedPointer =
        typename std::allocator_traits<Alloc>::rebind_traits<T>::pointer;

    using Primitive = detail::Primitive<Alloc>;

    Type _type;
    union {
        Primitive _primitive;
        AllocatedPointer<BasicString<Alloc>> _string;
        AllocatedPointer<BasicArray<Alloc>> _array;
        AllocatedPointer<BasicObject<Alloc>> _object;
    };

    template <class T>
    static auto _new(const auto &alloc, auto &&...args)
    {
        using NewAlloc = typename std::allocator_traits<Alloc>::rebind_alloc<T>;
        using NewAllocTraits = std::allocator_traits<NewAlloc>;

        NewAlloc new_alloc(alloc);

        auto p = NewAllocTraits::allocate(new_alloc, 1);

        NewAllocTraits::construct(
            new_alloc, std::to_address(p),
            std::forward<decltype(args)>(args)...);

        return p;
    }

    static void _delete(const auto &alloc, auto p) noexcept
    {
        using DeleteType = std::pointer_traits<decltype(p)>::element_type;
        using DeleteAlloc =
            typename std::allocator_traits<Alloc>::rebind_alloc<DeleteType>;
        using DeleteAllocTraits = std::allocator_traits<DeleteAlloc>;

        DeleteAlloc delete_alloc(alloc);

        DeleteAllocTraits::destroy(delete_alloc, std::to_address(p));
        DeleteAllocTraits::deallocate(delete_alloc, p, 1);
    }

    void _destroy() noexcept
    {
        switch (_type) {
        case Type::Null:
        case Type::Bool:
        case Type::Int:
        case Type::Float:
            _destroy_at_primitive();
            break;
        case Type::String:
            _delete(_string->get_allocator(), _string);
            _destroy_at_string();
            break;
        case Type::Array:
            _delete(_array->get_allocator(), _array);
            _destroy_at_array();
            break;
        case Type::Object:
            _delete(_object->get_allocator(), _object);
            _destroy_at_object();
            break;
        }
    }

    void _construct_at_primitive(auto &&...args)
    {
        std::construct_at(
            std::addressof(_primitive), std::forward<decltype(args)>(args)...);
    }

    void _construct_at_string(auto &&...args)
    {
        std::construct_at(
            std::addressof(_string), std::forward<decltype(args)>(args)...);
    }

    void _construct_at_array(auto &&...args)
    {
        std::construct_at(
            std::addressof(_array), std::forward<decltype(args)>(args)...);
    }

    void _construct_at_object(auto &&...args)
    {
        std::construct_at(
            std::addressof(_object), std::forward<decltype(args)>(args)...);
    }

    void _destroy_at_primitive() noexcept
    {
        std::destroy_at(std::addressof(_primitive));
    }

    void _destroy_at_string() noexcept
    {
        std::destroy_at(std::addressof(_string));
    }

    void _destroy_at_array() noexcept
    {
        std::destroy_at(std::addressof(_array));
    }

    void _destroy_at_object() noexcept
    {
        std::destroy_at(std::addressof(_object));
    }

    void _copy_construct(const BasicDocument &other, const Alloc &alloc)
    {
        switch (other._type) {
        case Type::Null:
        case Type::Bool:
        case Type::Int:
        case Type::Float:
            _construct_at_primitive(other._primitive, alloc);
            break;
        case Type::String:
            _construct_at_string(_new<BasicString<Alloc>>(
                alloc, *other._string, StringAlloc(alloc)));
            break;
        case Type::Array:
            _construct_at_array(_new<BasicArray<Alloc>>(
                alloc, *other._array, ArrayAlloc(alloc)));
            break;
        case Type::Object:
            _construct_at_object(_new<BasicObject<Alloc>>(
                alloc, *other._object, ObjectAlloc(alloc)));
            break;
        }

        _type = other._type;
    }

    void _move_construct(BasicDocument &&other) noexcept
    {
        switch (other._type) {
        case Type::Null:
        case Type::Bool:
        case Type::Int:
        case Type::Float:
            _construct_at_primitive(other._primitive);
            break;
        case Type::String:
            _construct_at_string(other._string);
            other._destroy_at_string();
            other._construct_at_primitive(Alloc(_string->get_allocator()));
            break;
        case Type::Array:
            _construct_at_array(other._array);
            other._destroy_at_array();
            other._construct_at_primitive(Alloc(_array->get_allocator()));
            break;
        case Type::Object:
            _construct_at_object(other._object);
            other._destroy_at_object();
            other._construct_at_primitive(Alloc(_object->get_allocator()));
            break;
        }

        _type = other._type;
        other._type = Type::Null;
    }

    void _copy_assign(const BasicDocument &other)
    {
        constexpr bool propagate = std::allocator_traits<
            Alloc>::propagate_on_container_copy_assignment::value;
        constexpr bool is_always_equal =
            std::allocator_traits<Alloc>::is_always_equal::value;

        if (_type == other._type &&
            (is_always_equal || get_allocator() == other.get_allocator())) {
            switch (_type) {
            case Type::Null:
            case Type::Bool:
            case Type::Int:
            case Type::Float:
                _primitive = other._primitive;
                break;
            case Type::String:
                *_string = *other._string;
                break;
            case Type::Array:
                *_array = *other._array;
                break;
            case Type::Object:
                *_object = *other._object;
                break;
            }
        } else {
            auto alloc = propagate ? get_allocator() : other.get_allocator();
            auto prev = std::move(*this);

            try {
                _destroy_at_primitive();
                _copy_construct(other, alloc);
            } catch (...) {
                _move_construct(std::move(prev));
                throw;
            }
        }
    }

    void _move_assign(BasicDocument &&other) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value ||
                 std::allocator_traits<
                     Alloc>::propagate_on_container_copy_assignment::value)
    {
        constexpr bool propagate = std::allocator_traits<
            Alloc>::propagate_on_container_move_assignment::value;
        constexpr bool is_always_equal =
            std::allocator_traits<Alloc>::is_always_equal::value;

        if (propagate || is_always_equal ||
            get_allocator() == other.get_allocator()) {
            _destroy();
            _move_construct(std::move(other));
        } else {
            auto alloc = propagate ? get_allocator() : other.get_allocator();
            auto prev = std::move(*this);

            try {
                _destroy_at_primitive();
                _copy_construct(other, alloc);
            } catch (...) {
                _move_construct(std::move(prev));
                throw;
            }
        }
    }

    void _assign_primitive(auto value, Type new_type) noexcept
    {
        switch (_type) {
        case Type::Null:
        case Type::Int:
        case Type::Bool:
        case Type::Float:
            _primitive.assign(value);
            break;
        case Type::String:
            _construct_at_primitive(value, _release_string());
            break;
        case Type::Array:
            _construct_at_primitive(value, _release_array());
            break;
        case Type::Object:
            _construct_at_primitive(value, _release_object());
            break;
        }

        _type = new_type;
    }

    void _assign_string(auto &&value)
    {
        if (is_string()) {
            *_string = std::forward<decltype(value)>(value);
        } else {
            emplace_string(std::forward<decltype(value)>(value));
        }
    }

    void _assign_array(auto &&value)
    {
        if (is_array()) {
            *_array = std::forward<decltype(value)>(value);
        } else {
            emplace_array(std::forward<decltype(value)>(value));
        }
    }

    void _assign_object(auto &&value)
    {
        if (is_object()) {
            *_object = std::forward<decltype(value)>(value);
        } else {
            emplace_object(std::forward<decltype(value)>(value));
        }
    }

    auto _release_string() noexcept
    {
        Alloc alloc(_string->get_allocator());
        _delete(alloc, _string);
        _destroy_at_string();
        return alloc;
    }

    auto _release_array() noexcept
    {
        Alloc alloc(_array->get_allocator());
        _delete(alloc, _array);
        _destroy_at_array();
        return alloc;
    }

    auto _release_object() noexcept
    {
        Alloc alloc(_object->get_allocator());
        _delete(alloc, _object);
        _destroy_at_object();
        return alloc;
    }
};

template <class Alloc>
class BasicArray {
private:
    using BaseValue = BasicDocument<Alloc>;
    using BaseAlloc =
        typename std::allocator_traits<Alloc>::rebind_alloc<BaseValue>;

    using Base = std::vector<BaseValue, BaseAlloc>;

    Base _base;

public:
    using value_type = BasicDocument<Alloc>;
    using allocator_type = Alloc;
    using pointer = Base::pointer;
    using const_pointer = Base::const_pointer;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = Base::iterator;
    using const_iterator = Base::const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    BasicArray() noexcept(noexcept(Alloc())) = default;

    explicit BasicArray(const Alloc &alloc) noexcept : _base(BaseAlloc(alloc))
    {}

    explicit BasicArray(size_type n, const Alloc &alloc = Alloc())
        : _base(n, BaseAlloc(alloc))
    {}

    BasicArray(size_type n, const_reference value, const Alloc &alloc = Alloc())
        : _base(n, value, BaseAlloc(alloc))
    {}

    template <std::input_iterator I, std::sentinel_for<I> S>
    BasicArray(I first, S last, const Alloc &alloc = Alloc())
        : _base(detail::make_common_iterator<I, S>(std::move(first)),
                detail::make_common_iterator<I, S>(std::move(last)),
                BaseAlloc(alloc))
    {}

    template <std::ranges::input_range R>
    BasicArray(R &&r, const Alloc &alloc = Alloc())
        : BasicArray(std::ranges::begin(r), std::ranges::end(r), alloc)
    {}

    BasicArray(const BasicArray &other) = default;

    BasicArray(const BasicArray &other, const Alloc &alloc)
        : _base(other._base, BaseAlloc(alloc))
    {}

    BasicArray(BasicArray &&other) noexcept = default;

    BasicArray(BasicArray &&other, const Alloc &alloc) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value)
        : _base(std::move(other._base), BaseAlloc(alloc))
    {}

    BasicArray(std::initializer_list<value_type> ilist,
               const Alloc &alloc = Alloc())
        : _base(ilist, BaseAlloc(alloc))
    {}

    ~BasicArray() noexcept = default;

    BasicArray &operator=(const BasicArray &other) = default;

    BasicArray &operator=(BasicArray &&other) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value ||
                 std::allocator_traits<Alloc>::
                     propagate_on_container_move_assignment::value) = default;

    BasicArray &operator=(std::initializer_list<value_type> ilist)
    {
        _base = ilist;
        return *this;
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    void assign(I first, S last)
    {
        _base.assign(detail::make_common_iterator<I, S>(std::move(first)),
                     detail::make_common_iterator<I, S>(std::move(last)));
    }

    template <std::ranges::input_range R>
    void assign(R &&r)
    {
        assign(std::ranges::begin(r), std::ranges::end(r));
    }

    void assign(size_type n, const_reference value)
    {
        _base.assign(n, value);
    }

    void assign(std::initializer_list<value_type> ilist)
    {
        _base.assign(ilist);
    }

    allocator_type get_allocator() const noexcept
    {
        return allocator_type(_base.get_allocator());
    }

    iterator begin() noexcept
    {
        return _base.begin();
    }

    const_iterator begin() const noexcept
    {
        return _base.begin();
    }

    iterator end() noexcept
    {
        return _base.end();
    }

    const_iterator end() const noexcept
    {
        return _base.end();
    }

    reverse_iterator rbegin() noexcept
    {
        return _base.rbegin();
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return _base.rbegin();
    }

    reverse_iterator rend() noexcept
    {
        return _base.rend();
    }

    const_reverse_iterator rend() const noexcept
    {
        return _base.rend();
    }

    const_iterator cbegin() const noexcept
    {
        return _base.cbegin();
    }

    const_iterator cend() const noexcept
    {
        return _base.cend();
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return _base.crbegin();
    }

    const_reverse_iterator crend() const noexcept
    {
        return _base.crend();
    }

    // capacity
    [[nodiscard]] bool empty() const noexcept
    {
        return _base.empty();
    }

    size_type size() const noexcept
    {
        return _base.size();
    }

    size_type max_size() const noexcept
    {
        return _base.max_size();
    }

    size_type capacity() const noexcept
    {
        return _base.capacity();
    }

    void resize(size_type new_size)
    {
        _base.resize(new_size);
    }

    void resize(size_type new_size, const_reference value)
    {
        _base.resize(new_size, value);
    }

    void reserve(size_type n)
    {
        _base.reserve(n);
    }

    void shrink_to_fit()
    {
        _base.shrink_to_fit();
    }

    void clear() noexcept
    {
        _base.clear();
    }

    reference operator[](size_type n)
    {
        return _base[n];
    }

    const_reference operator[](size_type n) const
    {
        return _base[n];
    }

    const_reference at(size_type n) const
    {
        return _base.at(n);
    }

    reference at(size_type n)
    {
        return _base.at(n);
    }

    reference front()
    {
        return _base.front();
    }

    const_reference front() const
    {
        return _base.front();
    }

    reference back()
    {
        return _base.back();
    }

    const_reference back() const
    {
        return _base.back();
    }

    pointer data() noexcept
    {
        return _base.data();
    }

    const_pointer data() const noexcept
    {
        return _base.data();
    }

    template <class... Args>
    reference emplace_back(Args &&...args)
    {
        return _base.emplace_back(std::forward<Args>(args)...);
    }

    void push_back(const_reference value)
    {
        _base.push_back(value);
    }

    void push_back(value_type &&value)
    {
        return _base.push_back(std::move(value));
    }

    void pop_back()
    {
        _base.pop_back();
    }

    template <class... Args>
    iterator emplace(const_iterator pos, Args &&...args)
    {
        return _base.emplace(pos, std::forward<Args>(args)...);
    }

    iterator insert(const_iterator pos, const_reference value)
    {
        return _base.insert(pos, value);
    }

    iterator insert(const_iterator pos, value_type &&value)
    {
        return _base.insert(pos, std::move(value));
    }

    iterator insert(const_iterator pos, size_type n, const_reference value)
    {
        return _base.insert(pos, n, value);
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    iterator insert(const_iterator pos, I first, S last)
    {
        return _base.insert(
            detail::make_common_iterator<I, S>(std::move(first)),
            detail::make_common_iterator<I, S>(std::move(last)));
    }

    template <std::ranges::input_range R>
    iterator insert(const_iterator pos, R &&r)
    {
        return insert(pos, std::ranges::begin(r), std::ranges::end(r));
    }

    iterator insert(const_iterator pos, std::initializer_list<value_type> ilist)
    {
        return _base.insert(pos, ilist);
    }

    iterator erase(const_iterator pos)
    {
        return _base.erase(pos);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        return _base.erase(first, last);
    }

    void swap(BasicArray &other) //
        noexcept(
            std::allocator_traits<Alloc>::is_always_equal::value ||
            std::allocator_traits<Alloc>::propagate_on_container_swap::value)
    {
        _base.swap(other._base);
    }

    friend bool operator==(const BasicArray &a, const BasicArray &b) = default;

    friend void swap(BasicArray &a, BasicArray &b) noexcept(noexcept(a.swap(b)))
    {
        a.swap(b);
    }
};

template <class Alloc>
class BasicObject {
private:
    using BaseKey = BasicString<Alloc>;
    using BaseMapped = BasicDocument<Alloc>;
    using BaseValue = std::pair<const BaseKey, BaseMapped>;
    using BaseHash = detail::StringViewHash;
    using BaseKeyEqual = detail::StringViewEqual;
    using BaseAlloc =
        typename std::allocator_traits<Alloc>::rebind_alloc<BaseValue>;
    using Base = std::unordered_map<
        BaseKey, BaseMapped, BaseHash, BaseKeyEqual, BaseAlloc>;

    Base _base;

public:
    // types
    using key_type = BasicString<Alloc>;
    using mapped_type = BasicDocument<Alloc>;
    using value_type =
        std::pair<const BasicString<Alloc>, BasicDocument<Alloc>>;
    using allocator_type = Alloc;
    using pointer = Base::pointer;
    using const_pointer = Base::const_pointer;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = Base::iterator;
    using const_iterator = Base::const_iterator;

    BasicObject() = default;

    explicit BasicObject(const Alloc &alloc) : _base(BaseAlloc(alloc)) {}

    explicit BasicObject(size_type bucket_count, const Alloc &alloc = Alloc())
        : _base(bucket_count, BaseAlloc(alloc))
    {}

    template <std::input_iterator I, std::sentinel_for<I> S>
    BasicObject(I first, S last, size_type bucket_count = 0,
                const Alloc &alloc = Alloc())
        : _base(detail::make_common_iterator<I, S>(std::move(first)),
                detail::make_common_iterator<I, S>(std::move(last)),
                bucket_count, BaseAlloc(alloc))
    {}

    template <std::input_iterator I, std::sentinel_for<I> S>
    BasicObject(I first, S last, const Alloc &alloc)
        : BasicObject(std::move(first), std::move(last), 0, alloc)
    {}

    template <std::ranges::input_range R>
    explicit BasicObject(
        R &&r, size_type bucket_count = 0, const Alloc &alloc = Alloc())
        : BasicObject(
              std::ranges::begin(r), std::ranges::end(r), bucket_count, alloc)
    {}

    template <std::ranges::input_range R>
    explicit BasicObject(R &&r, const Alloc &alloc)
        : BasicObject(std::forward<R>(r), 0, alloc)
    {}

    BasicObject(std::initializer_list<value_type> ilist,
                size_type bucket_count = 0, const Alloc &alloc = Alloc())
        : _base(ilist, bucket_count, BaseAlloc(alloc))
    {}

    BasicObject(std::initializer_list<value_type> ilist, const Alloc &alloc)
        : BasicObject(ilist, 0, alloc)
    {}

    BasicObject(const BasicObject &other) = default;

    BasicObject(const BasicObject &other, const Alloc &alloc)
        : _base(other._base, BaseAlloc(alloc))
    {}

    BasicObject(BasicObject &&other) = default;

    BasicObject(BasicObject &&other, const Alloc &alloc)
        : _base(std::move(other._base), BaseAlloc(alloc))
    {}

    ~BasicObject() = default;

    BasicObject &operator=(const BasicObject &other) = default;

    BasicObject &operator=(BasicObject &&other) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value) =
            default;

    BasicObject &operator=(std::initializer_list<value_type> ilist)
    {
        _base = ilist;
        return *this;
    }

    allocator_type get_allocator() const noexcept
    {
        return Alloc(_base.get_allocator());
    }

    iterator begin() noexcept
    {
        return _base.begin();
    }

    const_iterator begin() const noexcept
    {
        return _base.begin();
    }

    iterator end() noexcept
    {
        return _base.end();
    }

    const_iterator end() const noexcept
    {
        return _base.end();
    }

    const_iterator cbegin() const noexcept
    {
        return _base.cbegin();
    }

    const_iterator cend() const noexcept
    {
        return _base.cend();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return _base.empty();
    }

    size_type size() const noexcept
    {
        return _base.size();
    }

    size_type max_size() const noexcept
    {
        return _base.max_size();
    }

    template <class... Args>
    std::pair<iterator, bool> emplace(Args &&...args)
    {
        return _base.emplace(std::forward<Args>(args)...);
    }

    template <class... Args>
    iterator emplace_hint(const_iterator pos, Args &&...args)
    {
        return _base.emplace_hint(pos, std::forward<Args>(args)...);
    }

    std::pair<iterator, bool> insert(const value_type &value)
    {
        return _base.insert(value);
    }

    std::pair<iterator, bool> insert(value_type &&value)
    {
        return _base.insert(std::move(value));
    }

    template <class T>
    std::pair<iterator, bool> insert(T &&value)
    {
        return _base.insert(std::forward<T>(value));
    }

    iterator insert(const_iterator hint, const value_type &value)
    {
        return _base.insert(hint, value);
    }

    iterator insert(const_iterator hint, value_type &&value)
    {
        return _base.insert(hint, std::move(value));
    }

    template <class T>
    iterator insert(const_iterator hint, T &&value)
    {
        return _base.insert(hint, std::move(value));
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    void insert(I first, S last)
    {
        _base.insert(detail::make_common_iterator<I, S>(std::move(first)),
                     detail::make_common_iterator<I, S>(std::move(last)));
    }

    template <std::ranges::input_range R>
    void insert(R &&r)
    {
        insert(std::ranges::begin(r), std::ranges::end(r));
    }

    void insert(std::initializer_list<value_type> ilist)
    {
        _base.insert(ilist);
    }

    template <class... Args>
    std::pair<iterator, bool> try_emplace(const key_type &key, Args &&...args)
    {
        return _base.try_emplace(key, std::forward<Args>(args)...);
    }

    template <class... Args>
    std::pair<iterator, bool> try_emplace(key_type &&key, Args &&...args)
    {
        return _base.try_emplace(std::move(key), std::forward<Args>(args)...);
    }

    template <class... Args>
    iterator
    try_emplace(const_iterator hint, const key_type &key, Args &&...args)
    {
        return _base.try_emplace(hint, key, std::forward<Args>(args)...);
    }

    template <class... Args>
    iterator try_emplace(const_iterator hint, key_type &&key, Args &&...args)
    {
        return _base.try_emplace(
            hint, std::move(key), std::forward<Args>(args)...);
    }

    template <class T>
    std::pair<iterator, bool> insert_or_assign(const key_type &key, T &&value)
    {
        return _base.insert_or_assign(key, std::forward<T>(value));
    }

    template <class T>
    std::pair<iterator, bool> insert_or_assign(key_type &&key, T &&value)
    {
        return _base.insert_or_assign(std::move(key), std::forward<T>(value));
    }

    template <class T>
    iterator
    insert_or_assign(const_iterator hint, const key_type &key, T &&value)
    {
        return _base.insert_or_assign(hint, key, std::forward<T>(value));
    }

    template <class T>
    iterator insert_or_assign(const_iterator hint, key_type &&key, T &&value)
    {
        return _base.insert_or_assign(
            hint, std::move(key), std::forward<T>(value));
    }

    iterator erase(const_iterator pos)
    {
        return _base.erase(pos);
    }

    template <class K>
    size_type erase(K &&key)
    {
        return _base.erase(std::forward<K>(key));
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        return _base.erase(first, last);
    }

    void swap(BasicObject &other) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value)
    {
        _base.swap(other._base);
    }

    void clear() noexcept
    {
        _base.clear();
    }

    void merge(BasicObject &other)
    {
        _base.merge(other);
    }

    void merge(BasicObject &&other)
    {
        _base.merge(std::move(other));
    }

    iterator find(const key_type &key)
    {
        return _base.find(key);
    }

    const_iterator find(const key_type &key) const
    {
        return _base.find(key);
    }

    template <class K>
    iterator find(const K &key)
    {
        return _base.find(key);
    }

    template <class K>
    const_iterator find(const K &key) const
    {
        return _base.find(key);
    }

    template <class K>
    size_type count(const key_type &key) const
    {
        return _base.count(key);
    }

    template <class K>
    size_type count(const K &key) const
    {
        return _base.count(key);
    }

    bool contains(const key_type &key) const
    {
        return _base.contains(key);
    }

    template <class K>
    bool contains(const K &key) const
    {
        return _base.contains(key);
    }

    std::pair<iterator, iterator> equal_range(const key_type &key)
    {
        return _base.equal_range(key);
    }

    std::pair<const_iterator, const_iterator> equal_range(
        const key_type &key) const
    {
        return _base.equal_range(key);
    }

    template <class K>
    std::pair<iterator, iterator> equal_range(const K &key)
    {
        return _base.equal_range(key);
    }

    template <class K>
    std::pair<const_iterator, const_iterator> equal_range(const K &key) const
    {
        return _base.equal_range(key);
    }

    mapped_type &operator[](const key_type &key)
    {
        return _base[key];
    }

    mapped_type &operator[](key_type &&key)
    {
        return _base[std::move(key)];
    }

    mapped_type &at(const key_type &key)
    {
        return _base.at(key);
    }

    const mapped_type &at(const key_type &key) const
    {
        return _base.at(key);
    }

    void reserve(size_type n)
    {
        return _base.reserve(n);
    }

    friend bool operator==(const BasicObject &a, const BasicObject &b) =
        default;

    friend void swap(BasicObject &a, BasicObject &b) //
        noexcept(noexcept(a.swap(b)))
    {
        a.swap(b);
    }
};

template <class Alloc>
class BasicString {
private:
    using BaseAlloc = typename std::allocator_traits<Alloc>::rebind_alloc<char>;
    using Base = std::basic_string<char, std::char_traits<char>, BaseAlloc>;

    Base _base;

public:
    using traits_type = std::char_traits<char>;
    using value_type = char;
    using allocator_type = Alloc;
    using size_type = typename std::allocator_traits<Alloc>::size_type;
    using difference_type =
        typename std::allocator_traits<Alloc>::difference_type;
    using pointer = Base::pointer;
    using const_pointer = Base::const_pointer;
    using reference = value_type &;
    using const_reference = const value_type &;
    using iterator = Base::iterator;
    using const_iterator = Base::const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type npos = Base::npos;

    BasicString() noexcept(noexcept(Alloc())) : _base() {}

    explicit BasicString(const Alloc &alloc) noexcept : _base(alloc) {}

    BasicString(const BasicString &other, size_type pos,
                const Alloc &alloc = Alloc())
        : _base(other._base, pos, BaseAlloc(alloc))
    {}

    BasicString(const BasicString &other, size_type pos, size_type n,
                const Alloc &alloc = Alloc())
        : _base(other._base, pos, n, BaseAlloc(alloc))
    {}

    BasicString(BasicString &&other, size_type pos,
                const Alloc &alloc = Alloc())
        : _base(std::move(other._base), pos, BaseAlloc(alloc))
    {}

    BasicString(BasicString &&other, size_type pos, size_type n,
                const Alloc &alloc = Alloc())
        : _base(std::move(other), pos, n, BaseAlloc(alloc))
    {}

    template <StringViewLike T>
    explicit BasicString(const T &value, const Alloc &alloc = Alloc())
        : _base(value, BaseAlloc(alloc))
    {}

    template <StringViewLike T>
    BasicString(const T &value, size_type pos, size_type n,
                const Alloc &alloc = Alloc())
        : _base(value, pos, n, BaseAlloc(alloc))
    {}

    BasicString(const char *value, size_type n, const Alloc &alloc = Alloc())
        : _base(value, n, BaseAlloc(alloc))
    {}

    BasicString(const char *value, const Alloc &alloc = Alloc())
        : _base(value, BaseAlloc(alloc))
    {}

    BasicString(nullptr_t) = delete;

    BasicString(size_type n, char c, const Alloc &alloc = Alloc())
        : _base(n, c, BaseAlloc(alloc))
    {}

    template <std::input_iterator I, std::sentinel_for<I> S>
    BasicString(I first, S last, const Alloc &alloc = Alloc())
        : _base(detail::make_common_iterator<I, S>(std::move(first)),
                detail::make_common_iterator<I, S>(std::move(last)),
                BaseAlloc(alloc))
    {}

    BasicString(std::initializer_list<char> ilist, const Alloc &alloc = Alloc())
        : _base(ilist, BaseAlloc(alloc))
    {}

    BasicString(const BasicString &other) = default;

    BasicString(const BasicString &other, const Alloc &alloc)
        : _base(other, BaseAlloc(alloc))
    {}

    BasicString(BasicString &&other) noexcept = default;

    BasicString(BasicString &&other, const Alloc &alloc) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value)
        : _base(std::move(other._base), BaseAlloc(alloc))
    {}

    ~BasicString() noexcept = default;

    BasicString &operator=(const BasicString &other) = default;

    BasicString &operator=(BasicString &&other) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value ||
                 std::allocator_traits<Alloc>::
                     propagate_on_container_move_assignment::value) = default;

    template <StringViewLike T>
    BasicString &operator=(const T &value)
    {
        _base = value;
        return *this;
    }

    BasicString &operator=(const char *value)
    {
        _base = value;
        return *this;
    }

    BasicString &operator=(nullptr_t) = delete;

    BasicString &operator=(char c)
    {
        _base = c;
        return *this;
    }

    BasicString &operator=(std::initializer_list<char> ilist)
    {
        _base = ilist;
        return *this;
    }

    iterator begin() noexcept
    {
        return _base.begin();
    }

    const_iterator begin() const noexcept
    {
        return _base.begin();
    }

    iterator end() noexcept
    {
        return _base.end();
    }

    const_iterator end() const noexcept
    {
        return _base.end();
    }

    reverse_iterator rbegin() noexcept
    {
        return _base.rbegin();
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return _base.rbegin();
    }

    reverse_iterator rend() noexcept
    {
        return _base.rend();
    }

    const_reverse_iterator rend() const noexcept
    {
        return _base.rend();
    }

    const_iterator cbegin() const noexcept
    {
        return _base.cbegin();
    }

    const_iterator cend() const noexcept
    {
        return _base.cend();
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return _base.crbegin();
    }

    const_reverse_iterator crend() const noexcept
    {
        return _base.crend();
    }

    size_type size() const noexcept
    {
        return _base.size();
    }

    size_type length() const noexcept
    {
        return _base.length();
    }

    size_type max_size() const noexcept
    {
        return _base.max_size();
    }

    void resize(size_type n)
    {
        _base.resize(n);
    }

    void resize(size_type n, char c)
    {
        _base.resize(n, c);
    }

    size_type capacity() const noexcept
    {
        return _base.capacity();
    }

    void reserve(size_type n)
    {
        _base.reserve(n);
    }

    void shrink_to_fit()
    {
        _base.shrink_to_fit();
    }

    void clear() noexcept
    {
        _base.clear();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return _base.empty();
    }

    const_reference operator[](size_type n) const
    {
        return _base[n];
    }

    reference operator[](size_type n)
    {
        return _base[n];
    }

    const_reference at(size_type n) const
    {
        return _base.at(n);
    }

    reference at(size_type n)
    {
        return _base.at(n);
    }

    const char &front() const
    {
        return _base.front();
    }

    char &front()
    {
        return _base.front();
    }

    const char &back() const
    {
        return _base.back();
    }

    char &back()
    {
        return _base.back();
    }

    BasicString &operator+=(const BasicString &other)
    {
        _base += other._base;
        return *this;
    }

    template <StringViewLike T>
    BasicString &operator+=(const T &value)
    {
        _base += value;
        return *this;
    }

    BasicString &operator+=(const char *value)
    {
        _base += value;
        return *this;
    }

    BasicString &operator+=(char c)
    {
        _base += c;
        return *this;
    }

    BasicString &operator+=(std::initializer_list<char> ilist)
    {
        _base += ilist;
        return *this;
    }

    BasicString &append(const BasicString &other)
    {
        _base.append(other);
        return *this;
    }

    BasicString &append(const BasicString &other, size_type pos,
                        size_type n = npos)
    {
        _base.append(other, pos, n);
        return *this;
    }

    template <StringViewLike T>
    BasicString &append(const T &value)
    {
        _base.append(value);
        return *this;
    }

    template <StringViewLike T>
    BasicString &append(const T &value, size_type pos, size_type n = npos)
    {
        _base.append(value, pos, n);
        return *this;
    }

    BasicString &append(const char *value)
    {
        _base.append(value);
        return *this;
    }

    BasicString &append(const char *value, size_type n)
    {
        _base.append(value, n);
        return *this;
    }

    BasicString &append(size_type n, char c)
    {
        _base.append(n, c);
        return *this;
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    BasicString &append(I first, S last)
    {
        _base.append(detail::make_common_iterator<I, S>(std::move(first)),
                     detail::make_common_iterator<I, S>(std::move(last)));
        return *this;
    }

    BasicString &append(std::initializer_list<char> ilist)
    {
        _base.append(ilist);
        return *this;
    }

    void push_back(char c)
    {
        _base.push_back(c);
    }

    BasicString &assign(const BasicString &other)
    {
        _base.assign(other);
        return *this;
    }

    BasicString &assign(BasicString &&other) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value ||
                 std::allocator_traits<
                     Alloc>::propagate_on_container_move_assignment::value)
    {
        _base.assign(std::move(other));
        return *this;
    }

    BasicString &assign(const BasicString &other, size_type pos,
                        size_type n = npos)
    {
        _base.assign(other, pos, n);
        return *this;
    }

    template <StringViewLike T>
    BasicString &assign(const T &value)
    {
        _base.assign(value);
        return *this;
    }

    template <StringViewLike T>
    BasicString &assign(const T &value, size_type pos, size_type n = npos)
    {
        _base.assign(value, pos, n);
        return *this;
    }

    BasicString &assign(const char *value, size_type n)
    {
        _base.assign(value, n);
        return *this;
    }

    BasicString &assign(const char *value)
    {
        _base.assign(value);
        return *this;
    }

    BasicString &assign(size_type n, char c)
    {
        _base.assign(n, c);
        return *this;
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    BasicString &assign(I first, S last)
    {
        _base.assign(detail::make_common_iterator<I, S>(std::move(first)),
                     detail::make_common_iterator<I, S>(std::move(last)));
        return *this;
    }

    BasicString &assign(std::initializer_list<char> ilist)
    {
        _base.assign(ilist);
        return *this;
    }

    BasicString &insert(size_type pos, const BasicString &other)
    {
        _base.insert(pos, other._base);
        return *this;
    }

    BasicString &insert(size_type pos1, const BasicString &other,
                        size_type pos2, size_type n = npos)
    {
        _base.insert(pos1, other._base, pos2, n);
        return *this;
    }

    template <StringViewLike T>
    BasicString &insert(size_type pos, const T &value)
    {
        _base.insert(pos, value);
        return *this;
    }

    template <StringViewLike T>
    BasicString &
    insert(size_type pos1, const T &value, size_type pos2, size_type n = npos)
    {
        _base.insert(pos1, value, pos2, n);
        return *this;
    }

    BasicString &insert(size_type pos, const char *value, size_type n)
    {
        _base.insert(pos, value, n);
        return *this;
    }

    BasicString &insert(size_type pos, const char *value)
    {
        _base.insert(pos, value);
        return *this;
    }

    BasicString &insert(size_type pos, size_type n, char c)
    {
        _base.insert(pos, n, c);
        return *this;
    }

    iterator insert(const_iterator pos, char c)
    {
        return _base.insert(pos, c);
    }

    iterator insert(const_iterator pos, size_type n, char c)
    {
        return _base.insert(pos, n, c);
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    iterator insert(const_iterator pos, I first, S last)
    {
        return _base.insert(
            pos, detail::make_common_iterator<I, S>(std::move(first)),
            detail::make_common_iterator<I, S>(std::move(last)));
    }

    iterator insert(const_iterator pos, std::initializer_list<char> ilist)
    {
        return _base.insert(pos, ilist);
    }

    BasicString &erase(size_type pos = 0, size_type n = npos)
    {
        _base.erase(pos, n);
        return *this;
    }

    iterator erase(const_iterator pos)
    {
        return _base.erase(pos);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        return _base.erase(first, last);
    }

    void pop_back()
    {
        _base.pop_back();
    }

    BasicString &replace(size_type pos1, size_type n1, const BasicString &other)
    {
        _base.replace(pos1, n1, other._base);
        return *this;
    }

    BasicString &replace(size_type pos1, size_type n1, const BasicString &other,
                         size_type pos2, size_type n2 = npos)
    {
        _base.replace(pos1, n1, other._base, pos2, n2);
        return *this;
    }

    template <StringViewLike T>
    BasicString &replace(size_type pos1, size_type n1, const T &value)
    {
        _base.replace(pos1, n1, value);
        return *this;
    }

    template <StringViewLike T>
    BasicString &replace(size_type pos1, size_type n1, const T &value,
                         size_type pos2, size_type n2 = npos)
    {
        _base.replace(pos1, n1, value, pos2, n2);
        return *this;
    }

    BasicString &replace(
        size_type pos, size_type n1, const char *value, size_type n2)
    {
        _base.replace(pos, n1, value, n2);
        return *this;
    }

    BasicString &replace(size_type pos, size_type n1, const char *value)
    {
        _base.replace(pos, n1, value);
        return *this;
    }

    BasicString &replace(size_type pos, size_type n1, size_type n2, char c)
    {
        _base.replace(pos, n1, n2, c);
        return *this;
    }

    BasicString &replace(
        const_iterator first, const_iterator last, const BasicString &other)
    {
        _base.replace(first, last, other._base);
        return *this;
    }

    template <StringViewLike T>
    BasicString &
    replace(const_iterator first, const_iterator last, const T &value)
    {
        _base.replace(first, last, value);
        return *this;
    }

    BasicString &replace(const_iterator first, const_iterator last,
                         const char *value, size_type n)
    {
        _base.replace(first, last, value, n);
        return *this;
    }

    BasicString &replace(
        const_iterator first, const_iterator last, const char *value)
    {
        _base.replace(first, last, value);
        return *this;
    }

    BasicString &replace(
        const_iterator first, const_iterator last, size_type n, char c)
    {
        _base.replace(first, last, n, c);
        return *this;
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    BasicString &
    replace(const_iterator first1, const_iterator last1, I first2, S last2)
    {
        _base.replace(
            first1, last1,
            detail::make_common_iterator<I, S>(std::move(first2)),
            detail::make_common_iterator<I, S>(std::move(last2)));
        return *this;
    }

    BasicString &replace(const_iterator first, const_iterator last,
                         std::initializer_list<char> ilist)
    {
        _base.replace(first, last, ilist);
        return *this;
    }

    size_type copy(char *dest, size_type n, size_type pos = 0) const
    {
        _base.copy(dest, n, pos);
    }

    void swap(BasicString &other) //
        noexcept(
            std::allocator_traits<Alloc>::propagate_on_container_swap::value ||
            std::allocator_traits<Alloc>::is_always_equal::value)
    {
        _base.swap(other._base);
    }

    const char *c_str() const noexcept
    {
        return _base.c_str();
    }

    const char *data() const noexcept
    {
        return _base.data();
    }

    char *data() noexcept
    {
        return _base.data();
    }

    operator std::string_view() const noexcept
    {
        return std::string_view(_base);
    }

    allocator_type get_allocator() const noexcept
    {
        return Alloc(_base.get_allocator());
    }

    template <StringViewLike T>
    size_type find(const T &value, size_type pos = 0) const
        noexcept(std::is_nothrow_convertible_v<const T &, std::string_view>)
    {
        return _base.find(value, pos);
    }

    size_type find(const BasicString &other, size_type pos = 0) const noexcept
    {
        return _base.find(other._base, pos);
    }

    size_type find(const char *value, size_type pos, size_type n) const
    {
        return _base.find(pos, n);
    }

    size_type find(const char *value, size_type pos = 0) const
    {
        return _base.find(pos);
    }

    size_type find(char c, size_type pos = 0) const noexcept
    {
        return _base.find(c, pos);
    }

    template <StringViewLike T>
    size_type rfind(const T &value, size_type pos = npos) const
        noexcept(std::is_nothrow_convertible_v<const T &, std::string_view>)
    {
        return _base.rfind(value, pos);
    }

    size_type rfind(const BasicString &other,
                    size_type pos = npos) const noexcept
    {
        return _base.rfind(other._base, pos);
    }

    size_type rfind(const char *value, size_type pos, size_type n) const
    {
        return _base.rfind(value, pos, n);
    }

    size_type rfind(const char *value, size_type pos = npos) const
    {
        return _base.rfind(value, pos);
    }

    size_type rfind(char c, size_type pos = npos) const noexcept
    {
        return _base.rfind(c, pos);
    }

    template <StringViewLike T>
    size_type find_first_of(const T &value, size_type pos = 0) const
        noexcept(std::is_nothrow_convertible_v<const T &, std::string_view>)
    {
        return _base.find_first_of(value, pos);
    }

    size_type find_first_of(
        const BasicString &other, size_type pos = 0) const noexcept
    {
        return _base.find_first_of(other._base, pos);
    }

    size_type find_first_of(const char *value, size_type pos, size_type n) const
    {
        return _base.find_first_of(value, pos, n);
    }

    size_type find_first_of(const char *value, size_type pos = 0) const
    {
        return _base.find_first_of(value, pos);
    }

    size_type find_first_of(char c, size_type pos = 0) const noexcept
    {
        return _base.find_first_of(c, pos);
    }

    template <StringViewLike T>
    size_type find_last_of(const T &value, size_type pos = npos) const
        noexcept(std::is_nothrow_convertible_v<const T &, std::string_view>)
    {
        return _base.find_last_of(value, pos);
    }

    size_type find_last_of(
        const BasicString &other, size_type pos = npos) const noexcept
    {
        return _base.find_last_of(other._base, pos);
    }

    size_type find_last_of(const char *value, size_type pos, size_type n) const
    {
        return _base.find_last_of(value, pos, n);
    }

    size_type find_last_of(const char *value, size_type pos = npos) const
    {
        return _base.find_last_of(value, pos);
    }

    size_type find_last_of(char c, size_type pos = npos) const noexcept
    {
        return _base.find_last_of(c, pos);
    }

    template <StringViewLike T>
    size_type find_first_not_of(const T &value, size_type pos = 0) const
        noexcept(std::is_nothrow_convertible_v<const T &, std::string_view>)
    {
        return _base.find_first_not_of(value, pos);
    }

    size_type find_first_not_of(
        const BasicString &other, size_type pos = 0) const noexcept
    {
        return _base.find_first_not_of(other._base, pos);
    }

    size_type find_first_not_of(
        const char *value, size_type pos, size_type n) const
    {
        return _base.find_first_not_of(value, pos, n);
    }

    size_type find_first_not_of(const char *value, size_type pos = 0) const
    {
        return _base.find_first_not_of(value, pos);
    }

    size_type find_first_not_of(char c, size_type pos = 0) const noexcept
    {
        return _base.find_first_not_of(c, pos);
    }

    template <StringViewLike T>
    size_type find_last_not_of(const T &value, size_type pos = npos) const
        noexcept(std::is_nothrow_convertible_v<const T &, std::string_view>)
    {
        return _base.find_last_not_of(value, pos);
    }

    size_type find_last_not_of(
        const BasicString &other, size_type pos = npos) const noexcept
    {
        return _base.find_last_not_of(other._base, pos);
    }

    size_type find_last_not_of(
        const char *value, size_type pos, size_type n) const
    {
        return _base.find_last_not_of(value, pos, n);
    }

    size_type find_last_not_of(const char *value, size_type pos = npos) const
    {
        return _base.find_last_not_of(value, pos);
    }

    size_type find_last_not_of(char c, size_type pos = npos) const noexcept
    {
        return _base.find_last_not_of(c, pos);
    }

    BasicString substr(size_type pos = 0, size_type n = npos) const &
    {
        return _base.substr(pos, n);
    }

    BasicString substr(size_type pos = 0, size_type n = npos) &&
    {
        return std::move(_base).substr(pos, n);
    }

    template <StringViewLike T>
    int compare(const T &value) const
        noexcept(std::is_nothrow_convertible_v<const T &, std::string_view>)
    {
        return _base.compare(value);
    }

    template <StringViewLike T>
    int compare(size_type pos1, size_type n1, const T &value) const
    {
        return _base.compare(pos1, n1, value);
    }

    template <StringViewLike T>
    int compare(size_type pos1, size_type n1, const T &value, size_type pos2,
                size_type n2 = npos) const
    {
        return _base.compare(pos1, n1, value, pos2, n2);
    }

    int compare(const BasicString &other) const noexcept
    {
        return _base.compare(other._base);
    }

    int compare(size_type pos1, size_type n1, const BasicString &other) const
    {
        return _base.compare(pos1, n1, other._base);
    }

    int compare(size_type pos1, size_type n1, const BasicString &other,
                size_type pos2, size_type n2 = npos) const
    {
        return _base.compare(pos1, n1, other._base, pos2, n2);
    }

    int compare(const char *value) const
    {
        return _base.compare(value);
    }

    int compare(size_type pos1, size_type n1, const char *value) const
    {
        return _base.compare(pos1, n1, value);
    }

    int compare(size_type pos1, size_type n1, const char *value,
                size_type n2) const
    {
        return _base.compare(pos1, n1, value, n2);
    }

    bool starts_with(std::string_view sv) const noexcept
    {
        return _base.starts_with(sv);
    }

    bool starts_with(char c) const noexcept
    {
        return _base.starts_with(c);
    }

    bool starts_with(const char *value) const
    {
        return _base.starts_with(value);
    }

    bool ends_with(std::string_view sv) const noexcept
    {
        return _base.ends_with(sv);
    }

    bool ends_with(char c) const noexcept
    {
        return _base.ends_with(c);
    }

    bool ends_with(const char *value) const
    {
        return _base.ends_with(value);
    }

    bool contains(std::string_view sv) const noexcept
    {
        return _base.contains(sv);
    }

    bool contains(char c) const noexcept
    {
        return _base.ends_with(c);
    }

    bool contains(const char *value) const
    {
        return _base.ends_with(value);
    }

    friend bool operator==(const BasicString &a, const BasicString &b) =
        default;

    friend bool operator==(const BasicString &a, const char *b)
    {
        return a._base == b;
    }

    friend std::strong_ordering operator<=>(
        const BasicString &a, const BasicString &b) = default;

    friend std::strong_ordering operator<=>(const BasicString &a, const char *b)
    {
        return a._base <=> b;
    }

    friend void swap(BasicString &a, BasicString &b) //
        noexcept(noexcept(a.swap(b)))
    {
        a.swap(b);
    }

    BasicString operator+(const BasicString &a, const BasicString &b)
    {
        return a._base + b._base;
    }

    BasicString operator+(const BasicString &a, const char *b)
    {
        return a._base + b;
    }

    BasicString operator+(const char *a, const BasicString &b)
    {
        return a + b._base;
    }

    BasicString operator+(const BasicString &a, char b)
    {
        return a._base + b;
    }

    BasicString operator+(char a, const BasicString &b)
    {
        return a + b._base;
    }

    BasicString operator+(BasicString &&a, BasicString &&b)
    {
        return std::move(a._base) + std::move(b._base);
    }

    BasicString operator+(BasicString &&a, const BasicString &b)
    {
        return std::move(a._base) + b._base;
    }

    BasicString operator+(const BasicString &a, BasicString &&b)
    {
        return a._base + std::move(b._base);
    }

    BasicString operator+(BasicString &&a, const char *b)
    {
        return std::move(a._base) + b;
    }

    BasicString operator+(const char *a, BasicString &&b)
    {
        return a + std::move(b._base);
    }

    BasicString operator+(BasicString &&a, char b)
    {
        return std::move(a._base) + b;
    }

    BasicString operator+(char a, BasicString &&b)
    {
        return a + std::move(b._base);
    }

private:
    BasicString(Base &&value) noexcept : _base(std::move(value)) {}
};

struct ParseError {
public:
    ParseError() noexcept : ParseError(ParseErrorCode()) {}

    ParseError(ParseErrorCode code, int line = -1, int column = -1) noexcept
        : _code(code), _line(line), _column(column)
    {}

    ParseErrorCode code() const noexcept
    {
        return _code;
    }

    int line() const noexcept
    {
        return _line;
    }

    int column() const noexcept
    {
        return _column;
    }

    std::string_view message() const noexcept
    {
        switch (_code) {
        case ParseErrorCode::None:
            return "none";
        case ParseErrorCode::UnexpectedToken:
            return "unexpected token";
        case ParseErrorCode::InvalidEscape:
            return "invalid escape";
        case ParseErrorCode::InvalidEncoding:
            return "invalid encoding";
        case ParseErrorCode::MaxDepth:
            return "max depth reached";
        case ParseErrorCode::NumberOutOfRange:
            return "number out of range";
        case ParseErrorCode::DuplicateKey:
            return "duplicate key";
        default:
            return {};
        }
    }

    explicit operator bool() const noexcept
    {
        return static_cast<int>(_code) != 0;
    }

private:
    ParseErrorCode _code;
    int _line;
    int _column;
};

template <class I, class T>
struct ParseResult {
    [[no_unique_address]] I in;
    T value;
    ParseError error;

    template <class I2, class T2>
        requires std::convertible_to<const I &, I2> &&
                 std::convertible_to<const T &, T2>
    operator ParseResult<I2, T2>() const &
    {
        return { in, value, error };
    }

    template <class I2, class T2>
        requires std::convertible_to<I, I2> && std::convertible_to<T, T2>
    operator ParseResult<I2, T2>() &&
    {
        return { std::move(in), std::move(value), error };
    }
};

template <class Alloc>
class BasicParser {
public:
    BasicParser() noexcept(noexcept(Alloc())) : _alloc(), _opts() {}

    explicit BasicParser(const Alloc &alloc) noexcept : _alloc(alloc), _opts()
    {}

    explicit BasicParser(
        const ParseOptions &opts, const Alloc &alloc = Alloc()) noexcept
        : _alloc(alloc), _opts(opts)
    {}

    Alloc get_allocator() noexcept
    {
        return _alloc;
    }

    ParseOptions get_options() noexcept
    {
        return _opts;
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    ParseResult<I, BasicDocument<Alloc>> parse(I first, S last)
    {
        return parse(std::move(first), std::move(last), _alloc);
    }

    template <std::ranges::input_range R>
    ParseResult<std::ranges::borrowed_iterator_t<R>, BasicDocument<Alloc>>
    parse(R &&r)
    {
        return parse(std::ranges::begin(r), std::ranges::end(r));
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    ParseResult<I, BasicDocument<Alloc>>
    parse(I first, S last, const Alloc &alloc)
    {
        detail::ParseHandler<I, S, Alloc> handler(
            std::move(first), std::move(last), _alloc, alloc, _opts);

        return handler.parse();
    }

    template <std::ranges::input_range R>
    ParseResult<std::ranges::borrowed_iterator_t<R>, BasicDocument<Alloc>>
    parse(R &&r, const Alloc &alloc)
    {
        return parse(std::ranges::begin(r), std::ranges::end(r), alloc);
    }

private:
    [[no_unique_address]] Alloc _alloc;
    ParseOptions _opts;
};

template <std::input_iterator I, std::sentinel_for<I> S, class Alloc>
    requires(!std::same_as<Alloc, ParseOptions>)
inline ParseResult<I, BasicDocument<Alloc>> parse(
    I first, S last, const Alloc &alloc)
{
    return parse(std::move(first), std::move(last), {}, alloc);
}

template <std::input_iterator I, std::sentinel_for<I> S,
          class Alloc = std::allocator<std::byte>>
inline ParseResult<I, BasicDocument<Alloc>> parse(
    I first, S last, const ParseOptions &opts = ParseOptions(),
    const Alloc &alloc = Alloc())
{
    return BasicParser(opts, alloc).parse(std::move(first), std::move(last));
}

template <std::ranges::input_range R, class Alloc>
    requires(!std::same_as<Alloc, ParseOptions>)
inline ParseResult<std::ranges::borrowed_iterator_t<R>,
                   BasicDocument<Alloc>> parse(R &&r, const Alloc &alloc)
{
    return parse(std::forward<R>(r), {}, alloc);
}

template <std::ranges::input_range R, class Alloc = std::allocator<std::byte>>
inline ParseResult<std::ranges::borrowed_iterator_t<R>, BasicDocument<Alloc>>
parse(R &&r, const ParseOptions &opts = ParseOptions(),
      const Alloc &alloc = Alloc())
{
    return parse(std::ranges::begin(r), std::ranges::end(r), opts, alloc);
}

template <class Alloc>
class BasicSerializer {
public:
    BasicSerializer() noexcept(noexcept(Alloc())) : _alloc(), _opts() {}

    explicit BasicSerializer(const Alloc &alloc) noexcept
        : _alloc(alloc), _opts()
    {}

    explicit BasicSerializer(
        const SerializeOptions &opts, const Alloc &alloc = Alloc()) noexcept
        : _alloc(alloc), _opts(opts)
    {}

    template <std::output_iterator<char> O>
    O serialize(Null, O out)
    {
        return _serialize(nullptr, std::move(out));
    }

    template <std::output_iterator<char> O>
    O serialize(Bool value, O out)
    {
        return _serialize(value, std::move(out));
    }

    template <std::output_iterator<char> O>
    O serialize(std::integral auto value, O out)
    {
        return _serialize(value, std::move(out));
    }

    template <std::output_iterator<char> O>
    O serialize(std::floating_point auto value, O out)
    {
        return _serialize(value, std::move(out));
    }

    template <std::output_iterator<char> O>
    O serialize(const BasicString<Alloc> &value, O out)
    {
        return _serialize(value, std::move(out));
    }

    template <std::output_iterator<char> O>
    O serialize(const BasicArray<Alloc> &value, O out)
    {
        return _serialize(value, std::move(out));
    }

    template <std::output_iterator<char> O>
    O serialize(const BasicObject<Alloc> &value, O out)
    {
        return _serialize(value, std::move(out));
    }

    template <std::output_iterator<char> O>
    O serialize(const BasicDocument<Alloc> &value, O out)
    {
        return _serialize(value, std::move(out));
    }

private:
    Alloc _alloc;
    SerializeOptions _opts;

    template <class T, class O>
    O _serialize(const T &value, O out)
    {
        detail::SerializeHandler<O, Alloc> handler(
            std::move(out), _alloc, _opts);
        handler.serialize(value);
        return std::move(handler.out);
    }
};

template <class Alloc, std::output_iterator<char> O>
inline O serialize(Null, O out, const Alloc &alloc)
{
    return BasicSerializer(alloc).serialize(nullptr, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(
    Null, O out, const SerializeOptions &opts = SerializeOptions(),
    const Alloc &alloc = Alloc())
{
    return BasicSerializer(opts, alloc).serialize(nullptr, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(Bool value, O out, const Alloc &alloc)
{
    return BasicSerializer(alloc).serialize(value, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(
    Bool value, O out, const SerializeOptions &opts = SerializeOptions(),
    const Alloc &alloc = Alloc())
{
    return BasicSerializer(opts, alloc).serialize(value, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(std::integral auto value, O out, const Alloc &alloc)
{
    return BasicSerializer(alloc).serialize(value, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(
    std::integral auto value, O out,
    const SerializeOptions &opts = SerializeOptions(),
    const Alloc &alloc = Alloc())
{
    return BasicSerializer(opts, alloc).serialize(value, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(std::floating_point auto value, O out, const Alloc &alloc)
{
    return BasicSerializer(alloc).serialize(value, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(
    std::floating_point auto value, O out,
    const SerializeOptions &opts = SerializeOptions(),
    const Alloc &alloc = Alloc())
{
    return BasicSerializer(opts, alloc).serialize(value, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(const BasicString<Alloc> &value, O out, const Alloc &alloc)
{
    return BasicSerializer(alloc).serialize(value, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(
    const BasicString<Alloc> &value, O out,
    const SerializeOptions &opts = SerializeOptions(),
    const Alloc &alloc = Alloc())
{
    return BasicSerializer(opts, alloc).serialize(value, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(const BasicArray<Alloc> &value, O out, const Alloc &alloc)
{
    return BasicSerializer(alloc).serialize(value, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(
    const BasicArray<Alloc> &value, O out,
    const SerializeOptions &opts = SerializeOptions(),
    const Alloc &alloc = Alloc())
{
    return BasicSerializer(opts, alloc).serialize(value, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(const BasicObject<Alloc> &value, O out, const Alloc &alloc)
{
    return BasicSerializer(alloc).serialize(value, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(
    const BasicObject<Alloc> &value, O out,
    const SerializeOptions &opts = SerializeOptions(),
    const Alloc &alloc = Alloc())
{
    return BasicSerializer(opts, alloc).serialize(value, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(const BasicDocument<Alloc> &value, O out, const Alloc &alloc)
{
    return BasicSerializer(alloc).serialize(value, std::move(out));
}

template <class Alloc, std::output_iterator<char> O>
inline O serialize(
    const BasicDocument<Alloc> &value, O out,
    const SerializeOptions &opts = SerializeOptions(),
    const Alloc &alloc = Alloc())
{
    return BasicSerializer(opts, alloc).serialize(value, std::move(out));
}

template <std::output_iterator<char> O, class Alloc>
inline O to_chars(const BasicDocument<Alloc> &value, O out)
{
    return serialize(value, std::move(out));
}

template <std::output_iterator<char> O, class Alloc>
inline O to_chars(const BasicString<Alloc> &value, O out)
{
    return serialize(value, std::move(out));
}

template <std::output_iterator<char> O, class Alloc>
inline O to_chars(const BasicArray<Alloc> &value, O out)
{
    return serialize(value, std::move(out));
}

template <std::output_iterator<char> O, class Alloc>
inline O to_chars(const BasicObject<Alloc> &value, O out)
{
    return serialize(value, std::move(out));
}

template <class Alloc = std::allocator<char>, class ValueAlloc>
inline std::basic_string<char, std::char_traits<char>, Alloc>
to_string(const BasicDocument<ValueAlloc> &value, const Alloc &alloc = Alloc())
{
    std::basic_string<char, std::char_traits<char>, Alloc> dest(alloc);
    to_chars(value, std::back_inserter(dest));
    return dest;
}

template <class Alloc = std::allocator<char>, class ValueAlloc>
inline std::basic_string<char, std::char_traits<char>, Alloc>
to_string(const BasicString<ValueAlloc> &value, const Alloc &alloc = Alloc())
{
    std::basic_string<char, std::char_traits<char>, Alloc> dest(alloc);
    to_chars(value, std::back_inserter(dest));
    return dest;
}

template <class Alloc = std::allocator<char>, class ValueAlloc>
inline std::basic_string<char, std::char_traits<char>, Alloc>
to_string(const BasicArray<ValueAlloc> &value, const Alloc &alloc = Alloc())
{
    std::basic_string<char, std::char_traits<char>, Alloc> dest(alloc);
    to_chars(value, std::back_inserter(dest));
    return dest;
}

template <class Alloc = std::allocator<char>, class ValueAlloc>
inline std::basic_string<char, std::char_traits<char>, Alloc>
to_string(const BasicObject<ValueAlloc> &value, const Alloc &alloc = Alloc())
{
    std::basic_string<char, std::char_traits<char>, Alloc> dest(alloc);
    to_chars(value, std::back_inserter(dest));
    return dest;
}

template <class CharT, class Traits, class Alloc>
inline std::basic_ostream<CharT, Traits> &operator<<(
    std::basic_ostream<CharT, Traits> &stream,
    const BasicDocument<Alloc> &value)
{
    to_chars(value, std::ostream_iterator<char>(stream));
    return stream;
}

template <class CharT, class Traits, class Alloc>
inline std::basic_ostream<CharT, Traits> &operator<<(
    std::basic_ostream<CharT, Traits> &stream, const BasicString<Alloc> &value)
{
    to_chars(value, std::ostream_iterator<char>(stream));
    return stream;
}

template <class CharT, class Traits, class Alloc>
inline std::basic_ostream<CharT, Traits> &operator<<(
    std::basic_ostream<CharT, Traits> &stream, const BasicArray<Alloc> &value)
{
    to_chars(value, std::ostream_iterator<char>(stream));
    return stream;
}

template <class CharT, class Traits, class Alloc>
inline std::basic_ostream<CharT, Traits> &operator<<(
    std::basic_ostream<CharT, Traits> &stream, const BasicObject<Alloc> &value)
{
    to_chars(value, std::ostream_iterator<char>(stream));
    return stream;
}

template <class CharT, class Traits, class Alloc>
inline std::basic_istream<CharT, Traits> &operator>>(
    std::basic_istream<CharT, Traits> &stream, BasicDocument<Alloc> &dest)
{
    if (stream) {
        auto result = parse(
            std::istream_iterator<char>(stream), std::default_sentinel,
            dest.get_allocator());

        if (result.error) {
            stream.setstate(std::ios::failbit);
        } else {
            dest = std::move(result.dest);
        }
    }

    return stream;
}

template <class CharT, class Traits, class Alloc>
inline std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> &stream, BasicString<Alloc> &dest)
{
    if (!stream) {
        return stream;
    }

    BasicDocument<Alloc> document(dest.get_allocator());
    stream >> document;

    if (!stream) {
        return stream;
    } else if (!document.is_string()) {
        stream.setstate(std::ios::failbit);
        return stream;
    }

    dest = std::move(document.get_string());
    return stream;
}

template <class CharT, class Traits, class Alloc>
inline std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> &stream, BasicArray<Alloc> &dest)
{
    if (!stream) {
        return stream;
    }

    BasicDocument<Alloc> document(dest.get_allocator());
    stream >> document;

    if (!stream) {
        return stream;
    } else if (!document.is_array()) {
        stream.setstate(std::ios::failbit);
        return stream;
    }

    dest = std::move(document.get_array());
    return stream;
}

template <class CharT, class Traits, class Alloc>
inline std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> &stream, BasicObject<Alloc> &dest)
{
    if (!stream) {
        return stream;
    }

    BasicDocument<Alloc> document(dest.get_allocator());
    stream >> document;

    if (!stream) {
        return stream;
    } else if (!document.is_object()) {
        stream.setstate(std::ios::failbit);
        return stream;
    }

    dest = std::move(document.get_object());
    return stream;
}

} // namespace htl::json

#endif
