/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

#include "BitcoinExchange.hpp"

#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>

static inline int _maxDay(int year, int month)
{
    switch (month)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        return 31;
    case 2:
        if (year % 4 == 0 && year % 400 != 0)
        {
            return 29;
        }
        else
        {
            return 28;
        }
    case 4:
    case 6:
    case 9:
    case 11:
        return 30;
    }
    return 0;
}

static inline int _toDate(const std::string& s)
{
    std::istringstream iss(s);
    int year;
    int month;
    int day;
    char _;
    if (iss >> year && 0 <= year && year <= 9999 &&
        iss >> _ && _ == '-' &&
        iss >> month && 1 <= month && month <= 12 &&
        iss >> _ && _ == '-' &&
        iss >> day && 1 <= day && day <= _maxDay(year, month))
    {
        return (year * 100 + month) * 100 + day;
    }
    return 0;
}

static inline std::string _toDateString(int date)
{
    std::ostringstream oss;
    int day = date % 100;
    date /= 100;
    int month = date % 100;
    date /= 100;
    int year = date;
    oss << std::setfill('0') << std::setw(4) << year << '-'
        << std::setfill('0') << std::setw(2) << month << '-'
        << std::setfill('0') << std::setw(2) << day;
    return oss.str();
}

template <typename TFunc>
static inline bool _loadFile(const char* filename, std::string::value_type delim, TFunc func)
{
    std::ifstream ifs(filename);
    if (!ifs)
    {
        func(0, 0, "", -1);
        return false;
    }

    std::string line;
    // header
    if (!std::getline(ifs, line))
    {
        func(0, 0, "", -1);
        return false;
    }

    while (std::getline(ifs, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::istringstream iss(line);

        std::string dateStr;
        if (!std::getline(iss, dateStr, delim))
        {
            if (!func(0, 0, line, 1))
            {
                return false;
            }
            continue;
        }

        int date = _toDate(dateStr);
        if (date == 0)
        {
            if (!func(0, 0, line, 1))
            {
                return false;
            }
            continue;
        }

        double value;
        if (!(iss >> value))
        {
            if (!func(date, 0, line, 2))
            {
                return false;
            }
            continue;
        }

        if (!func(date, value, line, 0))
        {
            return false;
        }
    }

    return true;
}

BitcoinExchange::BitcoinExchange()
    : database()
{
}

BitcoinExchange::~BitcoinExchange()
{
}

struct InsertToDatabase
{
    std::map<int, double, std::greater<int> >& database;

    InsertToDatabase(std::map<int, double, std::greater<int> >& database)
        : database(database) {}

    InsertToDatabase(const InsertToDatabase& that)
        : database(that.database) {}

    ~InsertToDatabase() {}

    bool operator()(int date, double value, const std::string& line, int error)
    {
        static_cast<void>(line);
        return error == 0 && this->database.insert(std::make_pair(date, value)).second;
    }

private:
    InsertToDatabase();
    InsertToDatabase& operator=(const InsertToDatabase&);
};

bool BitcoinExchange::loadDatabase(const char* filename)
{
    return _loadFile(filename, ',', InsertToDatabase(this->database));
}

struct ValuePrinter
{
    const std::map<int, double, std::greater<int> >& database;

    ValuePrinter(const std::map<int, double, std::greater<int> >& database)
        : database(database) {}

    ValuePrinter(const ValuePrinter& that)
        : database(that.database) {}

    ~ValuePrinter() {}

    bool operator()(int date, double value, const std::string& line, int error)
    {
        if (error < 0)
        {
            std::cout << "Error: could not open file." << std::endl;
        }
        else if (error != 0)
        {
            std::cout << "Error: bad input => " << line << std::endl;
        }
        else if (value < 0)
        {
            std::cout << "Error: not a positive number." << std::endl;
        }
        else if (value > 1000)
        {
            std::cout << "Error: too large a number." << std::endl;
        }
        else
        {
            std::map<int, double, std::greater<int> >::const_iterator it = database.lower_bound(date);
            std::string dateStr = _toDateString(date);
            if (it != database.end())
            {
                std::cout << dateStr << " => " << value << " = " << value * it->second << std::endl;
            }
            else
            {
                std::cout << "Error: out of range date[" << dateStr << "]" << std::endl;
            }
        }

        return true;
    }

private:
    ValuePrinter();
    ValuePrinter& operator=(const ValuePrinter&);
};

void BitcoinExchange::printValue(const char* filename)
{
    static_cast<void>(_loadFile(filename, '|', ValuePrinter(this->database)));
}
