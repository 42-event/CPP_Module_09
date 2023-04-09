/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <list>
#include <sstream>
#include <vector>

#include <sys/time.h>

#include "timsort.hpp"

struct Stopwatch
{
public:
    static void _getTime(struct ::timespec& t)
    {
        clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    }

    static ::time_t _diffTime(struct ::timespec& t1, struct ::timespec& t2)
    {
        const ::time_t sec_diff = t2.tv_sec - t1.tv_sec;
        const ::time_t nsec_diff = t2.tv_nsec - t1.tv_nsec;
        return sec_diff * 1000000000L + nsec_diff;
    }

private:
    struct ::timespec start;

public:
    Stopwatch()
    {
        this->reset();
    }

    ::time_t splitLap()
    {
        struct ::timespec now;
        _getTime(now);
        return _diffTime(this->start, now);
    }

    void reset()
    {
        _getTime(this->start);
    }

    ~Stopwatch()
    {
    }

private:
    Stopwatch(const Stopwatch&);
    Stopwatch& operator=(const Stopwatch&);
};

namespace ft
{
    template <bool B, typename T>
    struct enable_if
    {
    };

    template <typename T>
    struct enable_if<true, T>
    {
        typedef T type;
    };

    template <typename T1 = void, typename T2 = void, typename T3 = void, typename T4 = void, typename T5 = void>
    struct make_void
    {
        typedef void type;
    };

    template <typename T, typename = void>
    struct has_const_iterator
    {
        static const bool value = false;
    };

    template <typename T>
    struct has_const_iterator<T, typename make_void<typename T::const_iterator>::type>
    {
        static const bool value = true;
    };

    template <typename T, typename = void>
    struct has_traits_type
    {
        static const bool value = false;
    };

    template <typename T>
    struct has_traits_type<T, typename make_void<typename T::traits_type>::type>
    {
        static const bool value = true;
    };
}

template <typename T>
typename ft::enable_if<ft::has_const_iterator<T>::value && !ft::has_traits_type<T>::value, std::ostream>::type& operator<<(std::ostream& os, const T& coll)
{
    std::ostringstream oss;
    for (typename T::const_iterator it = coll.begin(); it != coll.end(); ++it)
    {
        oss << ' ' << *it;
    }
    return os << oss.str();
}

int main(int argc, char* argv[])
{
    std::vector<int> iv;

    for (int i = 1; i < argc; i++)
    {
        int n = std::atoi(argv[i]);
        iv.push_back(n);
    }

    std::vector<int> first(iv.begin(), iv.end());
    std::list<int> second(iv.begin(), iv.end());

    std::cout << "Before:" << iv << std::endl;
    std::sort(iv.begin(), iv.end());
    std::cout << "After: " << iv << std::endl;

    Stopwatch sw;
    // TODO: PmergeMe first
    ft::timsort(first.begin(), first.end());
    ::time_t lap = sw.splitLap();
    if (std::equal(first.begin(), first.end(), iv.begin()))
    {
        std::cout << "Time to process a range of " << first.size() << " elements with std::vector : " << lap << "ns" << std::endl;
    }

    sw.reset();
    // TODO: PmergeMe second
    ft::timsort(second.begin(), second.end());
    lap = sw.splitLap();
    if (std::equal(second.begin(), second.end(), iv.begin()))
    {
        std::cout << "Time to process a range of " << second.size() << " elements with std::list :   " << lap << "ns" << std::endl;
    }

    return EXIT_SUCCESS;
}
