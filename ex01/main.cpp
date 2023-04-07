/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

#include "RPN.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        std::cout << "Error" << std::endl;
        return EXIT_FAILURE;
    }

    double value;
    if (!RPN::calculate(argv[1], value))
    {
        std::cout << "Error" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << value << std::endl;
    return EXIT_SUCCESS;
}
