#include <gtest/gtest.h>
#include <htl/scope_guard.h>

namespace htl::test {

TEST(ErrnoScopeGuardTest, MoveConstruct)
{
    errno = 1;

    {
        ErrnoScopeGuard guard;
        ASSERT_EQ(errno, 0);
        errno = 2;

        {
            ErrnoScopeGuard other(std::move(guard));
            ASSERT_EQ(errno, 2);
        }

        ASSERT_EQ(errno, 1);
        errno = 3;
    }

    ASSERT_EQ(errno, 3);
}

TEST(ErrnoScopeGuardTest, MoveAssign)
{
    errno = 1;

    {
        ErrnoScopeGuard guard;
        ASSERT_EQ(errno, 0);
        errno = 2;

        {
            ErrnoScopeGuard other;
            ASSERT_EQ(errno, 0);
            errno = 3;
            guard = std::move(other);
        }

        ASSERT_EQ(errno, 3);
    }

    ASSERT_EQ(errno, 2);
}

TEST(ErrnoScopeGuardTest, Swap)
{
    errno = 1;

    {
        ErrnoScopeGuard guard;
        ASSERT_EQ(errno, 0);
        errno = 2;

        {
            ErrnoScopeGuard other;
            ASSERT_EQ(errno, 0);
            errno = 3;
            std::ranges::swap(guard, other);
        }

        ASSERT_EQ(errno, 1);
    }

    ASSERT_EQ(errno, 2);
}

TEST(ErrnoScopeGuardTest, ResetsErrno)
{
    errno = 1;

    {
        ErrnoScopeGuard guard;
        ASSERT_EQ(errno, 0);
    }

    ASSERT_EQ(errno, 1);
}

TEST(ErrnoScopeGuardTest, Dismiss)
{
    errno = 1;

    {
        ErrnoScopeGuard guard;
        ASSERT_EQ(errno, 0);
        guard.dismiss();
    }

    ASSERT_EQ(errno, 0);
}

TEST(ErrnoScopeGuardTest, Value)
{
    errno = 1;
    ErrnoScopeGuard guard;
    ASSERT_EQ(guard.value(), 1);
}

} // namespace htl::test
