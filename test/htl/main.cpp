#include <clocale>
#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    std::cout << "LC_ALL: " << std::setlocale(LC_ALL, nullptr) << "\n";
    std::cout << "LC_COLLATE: " << std::setlocale(LC_COLLATE, nullptr) << "\n";
    std::cout << "LC_CTYPE: " << std::setlocale(LC_CTYPE, nullptr) << "\n";
    std::cout << "LC_MONETARY: " << std::setlocale(LC_MONETARY, nullptr)
              << "\n";
    std::cout << "LC_NUMERIC: " << std::setlocale(LC_NUMERIC, nullptr) << "\n";
    std::cout << "LC_TIME: " << std::setlocale(LC_TIME, nullptr) << "\n";
    return 0;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
