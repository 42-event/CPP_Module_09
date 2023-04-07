/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

#include "RPN.hpp"

#include <stack>
#include <string>

bool RPN::calculate(const std::string& str, double& value)
{
    std::stack<double> operands;

    const char* const delim = "\t\n\v\f\r ";
    for (std::string::size_type beg = str.find_first_not_of(delim), end; beg != std::string::npos; beg = str.find_first_not_of(delim, end))
    {
        end = str.find_first_of(delim, beg);
        if (beg != end)
        {
            std::string s = str.substr(beg, end - beg);
            if (s.empty())
            {
                continue;
            }

            if (s.length() != 1)
            {
                return false;
            }

            std::string::value_type c = s[0];
            if (std::strchr("+-*/", c))
            {
                if (operands.size() < 2)
                {
                    return false;
                }
                double rhs = operands.top();
                operands.pop();
                double lhs = operands.top();
                operands.pop();

                switch (c)
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
