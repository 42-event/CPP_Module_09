/* Any copyright is dedicated to the Public Domain.
 * https://creativecommons.org/publicdomain/zero/1.0/ */

#include <cstdlib>
#include <list>
#include <string>
#include <vector>

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

class PmergeMe
{
private:
    std::size_t run;

public:
    PmergeMe(std::size_t run);
    ~PmergeMe();

public:
    template <typename T>
    void insertionSort(std::vector<T>& c, typename std::vector<T>::iterator beg, typename std::vector<T>::iterator end);

    template <typename T>
    void insertionSort(std::list<T>& c, typename std::list<T>::iterator beg, typename std::list<T>::iterator end);

private:
    PmergeMe();
    PmergeMe(const PmergeMe&);
    PmergeMe& operator=(const PmergeMe&);
};

#include <ostream>
#include <sstream>

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

template <typename T>
void PmergeMe::insertionSort(std::vector<T>& c, typename std::vector<T>::iterator beg, typename std::vector<T>::iterator end)
{
    static_cast<void>(c);
    typedef typename std::vector<T>::difference_type diff_t;
    const diff_t n = end - beg;
    for (diff_t i = 1; i < n; i++)
    {
        typename std::vector<T>::value_type val = beg[i];
        diff_t j = i;
        while (--j >= 0)
        {
            if (!(val < beg[j]))
            {
                break;
            }
            beg[j + 1] = beg[j];
        }
        beg[j + 1] = val;
    }
}

template <typename T>
void PmergeMe::insertionSort(std::list<T>& c, typename std::list<T>::iterator beg, typename std::list<T>::iterator end)
{
    if (beg == end)
    {
        return;
    }
    typename std::list<T>::iterator pos = beg;
    ++pos;
    while (pos != end)
    {
        typename std::list<T>::value_type val = *pos;
        pos = c.erase(pos);
        typename std::list<T>::iterator it = beg;
        for (; it != pos; ++it)
        {
            if (val < *it)
            {
                break;
            }
        }
        typename std::list<T>::iterator inserted = c.insert(it, val);
        if (it == beg)
        {
            beg = inserted;
        }
    }
}
