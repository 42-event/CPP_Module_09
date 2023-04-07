/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

#include <string>

class RPN
{
public:
    static bool calculate(const std::string& str, double& value);

private:
    RPN();
    ~RPN();
    RPN(const RPN&);
    RPN& operator=(const RPN&);
};
