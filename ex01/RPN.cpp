/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

#include "RPN.hpp"

#include <cctype>
#include <cstring>
#include <stack>
#include <string>

static bool _do_operation(std::string::value_type operator_char, std::stack<double>& operands)
{
    if (operands.size() < 2)
    {
        return false;
    }

    double rhs = operands.top();
    operands.pop();
    double lhs = operands.top();
    operands.pop();

    switch (operator_char)
    {
    case '+':
        operands.push(lhs + rhs);
        break;
    case '-':
        operands.push(lhs - rhs);
        break;
    case '*':
        operands.push(lhs * rhs);
        break;
    case '/':
        if (rhs == 0)
        {
            // !!
            return false;
        }
        operands.push(lhs / rhs);
        break;
    }
    return true;
}

bool RPN::calculate(const std::string& str, double& value)
{
    std::stack<double> operands;

    const char* const delim = "\t\n\v\f\r ";
    for (std::string::size_type beg = str.find_first_not_of(delim), end; beg != std::string::npos; beg = str.find_first_not_of(delim, end))
    {
        end = str.find_first_of(delim, beg);
        if (beg != end)
        {
            if (end - beg != sizeof(std::string::value_type))
            {
                return false;
            }

            const std::string::value_type c = str[beg];
            if (std::strchr("+-*/", c))
            {
                if (!_do_operation(c, operands))
                {
                    return false;
                }
            }
            else if (std::isdigit(c))
            {
                operands.push(c - '0');
            }
            else
            {
                return false;
            }
        }
    }

    if (operands.size() != 1)
    {
        return false;
    }

    value = operands.top();
    return true;
}
