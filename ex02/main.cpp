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

    template <typename T>
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
typename ft::enable_if<ft::has_const_iterator<T>::value && !ft::has_traits_type<T>::value, std::ostream>::type& operator<<(std::ostream& os, const T& vec)
{
    std::ostringstream oss;
    for (typename T::const_iterator it = vec.begin(); it != vec.end(); ++it)
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
    ::time_t lap = sw.splitLap();
    std::cout << "Time to process a range of " << first.size() << " elements with std::vector : " << lap << "ns" << std::endl;

    sw.reset();
    // TODO: PmergeMe second
    lap = sw.splitLap();
    std::cout << "Time to process a range of " << second.size() << " elements with std::list : " << lap << "ns" << std::endl;

    return EXIT_SUCCESS;
}
