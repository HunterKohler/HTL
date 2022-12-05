#ifndef HTL_TEST_ALLOCATOR_H_
#define HTL_TEST_ALLOCATOR_H_

#include <memory>
#include <unordered_map>

namespace htl::test {

template <class T>
struct DefaultInitAllocator {
    using value_type = T;

    DefaultInitAllocator() = default;

    template <class U>
    DefaultInitAllocator(const DefaultInitAllocator<U> &other) noexcept
        : state{ other.state }
    {}

    T *allocate(std::size_t n)
    {
        return std::allocator<T>{}.allocate(n);
    }

    void deallocate(T *p, std::size_t n)
    {
        return std::allocator<T>{}.deallocate(p, n);
    }

    int state;
};

struct IdentityAllocatorBase {
    using map_type = std::unordered_map<void *, int>;

    static map_type &get_map()
    {
        static map_type map;
        return map;
    }
};

template <class T>
struct IdentityAllocator : IdentityAllocatorBase {
    using value_type = T;
    using is_always_equal = std::false_type;

    template <class U>
    struct rebind {
        using other = IdentityAllocator<U>;
    };

    IdentityAllocator() noexcept : id{} {}
    IdentityAllocator(int id) noexcept : id{ id } {}

    template <class U>
    IdentityAllocator(const IdentityAllocator<U> &other) noexcept
        : id{ other.id }
    {}

    T *allocate(std::size_t n)
    {
        T *p = std::allocator<T>{}.allocate(n);

        try {
            get_map().emplace(p, id);
        } catch (...) {
            deallocate(p, n);
            throw;
        }

        return p;
    }

    void deallocate(T *p, std::size_t n)
    {
        // Assert: p

        map_type::iterator it = get_map().find(p);

        assert(it != get_map().end());
        assert(it->second == id);

        std::allocator<T>{}.deallocate(p, n);
    }

    int id;

    friend bool operator==(
        const IdentityAllocator &a, const IdentityAllocator &b) noexcept
    {
        return a.id == b.id;
    }
};

template <class T, bool Propagate>
struct PropagatingAllocator : IdentityAllocator<T> {
    using propagate = std::bool_constant<Propagate>;
    using propagate_on_container_move_assignment = propagate;
    using propagate_on_container_copy_assignment = propagate;
    using propagate_on_container_swap = propagate;

    template <class U>
    struct rebind {
        using other = PropagatingAllocator<U, Propagate>;
    };

    PropagatingAllocator() noexcept = default;
    PropagatingAllocator(int id) noexcept : IdentityAllocator<T>{ id } {}

    template <class U>
    PropagatingAllocator(const PropagatingAllocator<U, Propagate> &other) //
        noexcept
        : IdentityAllocator<T>{ other }
    {}

    PropagatingAllocator(const PropagatingAllocator &other) noexcept = default;
    PropagatingAllocator(PropagatingAllocator &&other) noexcept = default;

    PropagatingAllocator &operator=(const PropagatingAllocator &) //
        requires(Propagate)
    = default;

    PropagatingAllocator &operator=(PropagatingAllocator &&) noexcept = default;

    PropagatingAllocator select_on_container_copy_construction() const
    {
        return Propagate ? *this : PropagatingAllocator{};
    }
};

} // namespace htl::test

#endif
