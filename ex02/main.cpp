/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

#include "PmergeMe.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <list>
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

int main(int argc, char* argv[])
{
    std::vector<int> iv;

    for (int i = 1; i < argc; i++)
    {
        int n = std::atoi(argv[i]);
        iv.push_back(n);
    }

    PmergeMe sort(16);
    std::vector<int> first(iv.begin(), iv.end());
    std::list<int> second(iv.begin(), iv.end());

    std::cout << "Before:" << iv << std::endl;
    std::sort(iv.begin(), iv.end());
    std::cout << "After: " << iv << std::endl;

    Stopwatch sw;
    // TODO: PmergeMe first
    sort.insertionSort(first, first.begin(), first.end());
    ::time_t lap = sw.splitLap();
    if (std::equal(first.begin(), first.end(), iv.begin()))
    {
        std::cout << "Time to process a range of " << first.size() << " elements with std::vector : " << lap << "ns" << std::endl;
    }
    else
    {
        std::cout << "WhatTheHell: " << first << std::endl;
    }

    sw.reset();
    // TODO: PmergeMe second
    sort.insertionSort(second, second.begin(), second.end());
    lap = sw.splitLap();
    if (std::equal(second.begin(), second.end(), iv.begin()))
    {
        std::cout << "Time to process a range of " << second.size() << " elements with std::list :   " << lap << "ns" << std::endl;
    }
    else
    {
        std::cout << "WhatTheFuck: " << second << std::endl;
    }

    return EXIT_SUCCESS;
}
