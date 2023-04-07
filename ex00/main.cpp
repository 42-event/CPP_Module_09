/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

#include "BitcoinExchange.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        std::cout << "Error: could not open file." << std::endl;
        return EXIT_FAILURE;
    }

    BitcoinExchange exchange;
    if (!exchange.loadDatabase())
    {
        std::cout << "Error: could not load database." << std::endl;
        return EXIT_FAILURE;
    }

    exchange.printValue(argv[1]);
    return EXIT_SUCCESS;
}
