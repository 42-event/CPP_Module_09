/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

#include <functional>
#include <map>
#include <string>

class BitcoinExchange
{
private:
    std::map<int, double, std::greater<int> > database;

public:
    BitcoinExchange();
    ~BitcoinExchange();

public:
    bool loadDatabase(const char* filename = "data.csv");
    void printValue(const char* filename = "input.txt");

private:
    BitcoinExchange(const BitcoinExchange&);
    BitcoinExchange& operator=(const BitcoinExchange&);
};
