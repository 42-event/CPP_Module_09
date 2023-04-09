/*
 * C++ implementation of timsort
 *
 * ported from Python's and OpenJDK's:
 * - http://svn.python.org/projects/python/trunk/Objects/listobject.c
 * - http://cr.openjdk.java.net/~martin/webrevs/openjdk7/timsort/raw_files/new/src/share/classes/java/util/TimSort.java
 *
 * Copyright (c) 2011 Fuji, Goro (gfx) <gfuji@cpan.org>.
 * Copyright (c) 2019-2021 Morwenn.
 * Copyright (c) 2021 Igor Kushnir <igorkuo@gmail.com>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>

#include <cassert>

// Diagnostic selection macros

#ifdef _TIMSORT_ENABLE_LOG
#include <iostream>
#define _TIMSORT_LOG(expr) (std::clog << "# " << __func__ << ": " << expr << std::endl)
#else
#define _TIMSORT_LOG(expr) ((void)0)
#endif

namespace ft
{

    // ---------------------------------------
    // Implementation details
    // ---------------------------------------

    namespace detail
    {

        template <typename Iterator>
        struct run
        {
            typedef typename std::iterator_traits<Iterator>::difference_type diff_t;

            Iterator base;
            diff_t len;

            run(Iterator b, diff_t l) : base(b), len(l)
            {
            }
        };

        template <typename RandomAccessIterator, typename Compare>
        class TimSort
        {
            typedef RandomAccessIterator iter_t;
            typedef typename std::iterator_traits<iter_t>::value_type value_t;
            typedef typename std::iterator_traits<iter_t>::reference ref_t;
            typedef typename std::iterator_traits<iter_t>::difference_type diff_t;

            static const int MIN_MERGE = 32;
            static const int MIN_GALLOP = 7;

            int minGallop_; // default to MIN_GALLOP

            std::vector<value_t> tmp_; // temp storage for merges
            typedef typename std::vector<value_t>::iterator tmp_iter_t;

            std::vector<run<RandomAccessIterator> > pending_;

            template <class InputIt>
            static InputIt next(InputIt it, typename std::iterator_traits<InputIt>::difference_type n = 1)
            {
                return std::advance(it, n), it;
            }

            static void binarySort(iter_t const lo, iter_t const hi, iter_t start, Compare compare)
            {
                // // assert(lo <= start);
                // // assert(start <= hi);
                if (start == lo)
                {
                    ++start;
                }
                for (; start != hi; ++start)
                {
                    // // assert(lo <= start);
                    value_t pivot = *start;

                    iter_t const pos = std::upper_bound(lo, start, pivot, compare);
                    for (iter_t p = start; p != pos; --p)
                    {
                        *p = *next(p, -1);
                    }
                    *pos = pivot;
                }
            }

            static diff_t countRunAndMakeAscending(iter_t const lo, iter_t const hi, Compare compare)
            {
                // assert(lo < hi);

                iter_t runHi = next(lo, 1);
                if (runHi == hi)
                {
                    return 1;
                }

                if (compare(*runHi, *lo))
                {
                    // decreasing
                    do
                    {
                        ++runHi;
                    } while (runHi != hi && compare(*runHi, *next(runHi, -1)));
                    std::reverse(lo, runHi);
                }
                else
                {
                    // non-decreasing
                    do
                    {
                        ++runHi;
                    } while (runHi != hi && !compare(*runHi, *next(runHi, -1)));
                }

                return std::distance(lo, runHi);
            }

            static diff_t minRunLength(diff_t n)
            {
                // assert(n >= 0);

                diff_t r = 0;
                while (n >= 2 * MIN_MERGE)
                {
                    r |= (n & 1);
                    n >>= 1;
                }
                return n + r;
            }

            TimSort() : minGallop_(MIN_GALLOP)
            {
            }

            // Silence GCC -Winline warning
            ~TimSort() {}

            void pushRun(iter_t const runBase, diff_t const runLen)
            {
                pending_.push_back(run<iter_t>(runBase, runLen));
            }

            void mergeCollapse(Compare compare)
            {
                while (pending_.size() > 1)
                {
                    diff_t n = pending_.size() - 2;

                    if ((n > 0 && pending_[n - 1].len <= pending_[n].len + pending_[n + 1].len) ||
                        (n > 1 && pending_[n - 2].len <= pending_[n - 1].len + pending_[n].len))
                    {
                        if (pending_[n - 1].len < pending_[n + 1].len)
                        {
                            --n;
                        }
                        mergeAt(n, compare);
                    }
                    else if (pending_[n].len <= pending_[n + 1].len)
                    {
                        mergeAt(n, compare);
                    }
                    else
                    {
                        break;
                    }
                }
            }

            void mergeForceCollapse(Compare compare)
            {
                while (pending_.size() > 1)
                {
                    diff_t n = pending_.size() - 2;

                    if (n > 0 && pending_[n - 1].len < pending_[n + 1].len)
                    {
                        --n;
                    }
                    mergeAt(n, compare);
                }
            }

            void mergeAt(diff_t const i, Compare compare)
            {
                diff_t const stackSize = pending_.size();
                // assert(stackSize >= 2);
                // assert(i >= 0);
                // assert(i == stackSize - 2 || i == stackSize - 3);

                iter_t base1 = pending_[i].base;
                diff_t len1 = pending_[i].len;
                iter_t base2 = pending_[i + 1].base;
                diff_t len2 = pending_[i + 1].len;

                pending_[i].len = len1 + len2;

                if (i == stackSize - 3)
                {
                    pending_[i + 1] = pending_[i + 2];
                }

                pending_.pop_back();

                mergeConsecutiveRuns(base1, len1, base2, len2, compare);
            }

            void mergeConsecutiveRuns(iter_t base1, diff_t len1, iter_t base2, diff_t len2, Compare compare)
            {
                // assert(len1 > 0);
                // assert(len2 > 0);
                // assert(base1 + len1 == base2);

                diff_t const k = gallopRight(*base2, base1, len1, 0, compare);
                // assert(k >= 0);

                std::advance(base1, k);
                len1 -= k;

                if (len1 == 0)
                {
                    return;
                }

                len2 = gallopLeft(*next(base1, len1 - 1), base2, len2, len2 - 1, compare);
                // assert(len2 >= 0);
                if (len2 == 0)
                {
                    return;
                }

                if (len1 <= len2)
                {
                    mergeLo(base1, len1, base2, len2, compare);
                }
                else
                {
                    mergeHi(base1, len1, base2, len2, compare);
                }
            }

            template <typename Iter>
            static diff_t gallopLeft(ref_t key, Iter const base, diff_t const len,
                                     diff_t const hint, Compare compare)
            {
                // assert(len > 0);
                // assert(hint >= 0);
                // assert(hint < len);

                diff_t lastOfs = 0;
                diff_t ofs = 1;

                if (compare(*next(base, hint), key))
                {
                    diff_t const maxOfs = len - hint;
                    while (ofs < maxOfs && compare(*next(base, hint + ofs), key))
                    {
                        lastOfs = ofs;
                        ofs = (ofs << 1) + 1;

                        if (ofs <= 0)
                        {
                            // int overflow
                            ofs = maxOfs;
                        }
                    }
                    if (ofs > maxOfs)
                    {
                        ofs = maxOfs;
                    }

                    lastOfs += hint;
                    ofs += hint;
                }
                else
                {
                    diff_t const maxOfs = hint + 1;
                    while (ofs < maxOfs && !compare(*next(base, hint - ofs), key))
                    {
                        lastOfs = ofs;
                        ofs = (ofs << 1) + 1;

                        if (ofs <= 0)
                        {
                            ofs = maxOfs;
                        }
                    }
                    if (ofs > maxOfs)
                    {
                        ofs = maxOfs;
                    }

                    diff_t const tmp = lastOfs;
                    lastOfs = hint - ofs;
                    ofs = hint - tmp;
                }
                // assert(-1 <= lastOfs);
                // assert(lastOfs < ofs);
                // assert(ofs <= len);

                return std::distance(base, std::lower_bound(next(base, lastOfs + 1), next(base, ofs), key, compare));
            }

            template <typename Iter>
            static diff_t gallopRight(ref_t key, Iter const base, diff_t const len,
                                      diff_t const hint, Compare compare)
            {
                // assert(len > 0);
                // assert(hint >= 0);
                // assert(hint < len);

                diff_t ofs = 1;
                diff_t lastOfs = 0;

                if (compare(key, *next(base, hint)))
                {
                    diff_t const maxOfs = hint + 1;
                    while (ofs < maxOfs && compare(key, *next(base, hint - ofs)))
                    {
                        lastOfs = ofs;
                        ofs = (ofs << 1) + 1;

                        if (ofs <= 0)
                        {
                            ofs = maxOfs;
                        }
                    }
                    if (ofs > maxOfs)
                    {
                        ofs = maxOfs;
                    }

                    diff_t const tmp = lastOfs;
                    lastOfs = hint - ofs;
                    ofs = hint - tmp;
                }
                else
                {
                    diff_t const maxOfs = len - hint;
                    while (ofs < maxOfs && !compare(key, *next(base, hint + ofs)))
                    {
                        lastOfs = ofs;
                        ofs = (ofs << 1) + 1;

                        if (ofs <= 0)
                        {
                            // int overflow
                            ofs = maxOfs;
                        }
                    }
                    if (ofs > maxOfs)
                    {
                        ofs = maxOfs;
                    }

                    lastOfs += hint;
                    ofs += hint;
                }
                // assert(-1 <= lastOfs);
                // assert(lastOfs < ofs);
                // assert(ofs <= len);

                return std::distance(base, std::upper_bound(next(base, (lastOfs + 1)), next(base, ofs), key, compare));
            }

            static void rotateLeft(iter_t first, iter_t last)
            {
                value_t tmp = *first;
                iter_t last_1 = std::copy(next(first, 1), last, first);
                *last_1 = tmp;
            }

            static void rotateRight(iter_t first, iter_t last)
            {
                iter_t last_1 = next(last, -1);
                value_t tmp = *last_1;
                std::copy_backward(first, last_1, last);
                *first = tmp;
            }

            void mergeLo(iter_t const base1, diff_t len1, iter_t const base2, diff_t len2, Compare compare)
            {
                // assert(len1 > 0);
                // assert(len2 > 0);
                // assert(base1 + len1 == base2);

                if (len1 == 1)
                {
                    return rotateLeft(base1, next(base2, len2));
                }
                if (len2 == 1)
                {
                    return rotateRight(base1, next(base2, len2));
                }

                copy_to_tmp(base1, len1);

                tmp_iter_t cursor1 = tmp_.begin();
                iter_t cursor2 = base2;
                iter_t dest = base1;

                *dest = *cursor2;
                ++cursor2;
                ++dest;
                --len2;

                int minGallop(minGallop_);

                // outer:
                while (true)
                {
                    diff_t count1 = 0;
                    diff_t count2 = 0;

                    do
                    {
                        // assert(len1 > 1);
                        // assert(len2 > 0);

                        if (compare(*cursor2, *cursor1))
                        {
                            *dest = *cursor2;
                            ++cursor2;
                            ++dest;
                            ++count2;
                            count1 = 0;
                            if (--len2 == 0)
                            {
                                goto epilogue;
                            }
                        }
                        else
                        {
                            *dest = *cursor1;
                            ++cursor1;
                            ++dest;
                            ++count1;
                            count2 = 0;
                            if (--len1 == 1)
                            {
                                goto epilogue;
                            }
                        }
                    } while ((count1 | count2) < minGallop);

                    do
                    {
                        // assert(len1 > 1);
                        // assert(len2 > 0);

                        count1 = gallopRight(*cursor2, cursor1, len1, 0, compare);
                        if (count1 != 0)
                        {
                            std::copy_backward(cursor1, cursor1 + count1, next(dest, count1));
                            std::advance(dest, count1);
                            std::advance(cursor1, count1);
                            len1 -= count1;

                            if (len1 <= 1)
                            {
                                goto epilogue;
                            }
                        }
                        *dest = *cursor2;
                        ++cursor2;
                        ++dest;
                        if (--len2 == 0)
                        {
                            goto epilogue;
                        }

                        count2 = gallopLeft(*cursor1, cursor2, len2, 0, compare);
                        if (count2 != 0)
                        {
                            std::copy(cursor2, next(cursor2, count2), dest);
                            std::advance(dest, count2);
                            std::advance(cursor2, count2);
                            len2 -= count2;
                            if (len2 == 0)
                            {
                                goto epilogue;
                            }
                        }
                        *dest = *cursor1;
                        ++cursor1;
                        ++dest;
                        if (--len1 == 1)
                        {
                            goto epilogue;
                        }

                        --minGallop;
                    } while ((count1 >= MIN_GALLOP) | (count2 >= MIN_GALLOP));

                    if (minGallop < 0)
                    {
                        minGallop = 0;
                    }
                    minGallop += 2;
                } // end of "outer" loop

            epilogue: // merge what is left from either cursor1 or cursor2

                minGallop_ = (std::min)(minGallop, 1);

                if (len1 == 1)
                {
                    // assert(len2 > 0);
                    std::copy(cursor2, next(cursor2, len2), dest);
                    *next(dest, len2) = *cursor1;
                }
                else
                {
                    // assert(len1 != 0 && "Comparison function violates its general contract");
                    // assert(len2 == 0);
                    // assert(len1 > 1);
                    std::copy(cursor1, next(cursor1, len1), dest);
                }
            }

            void mergeHi(iter_t const base1, diff_t len1, iter_t const base2, diff_t len2, Compare compare)
            {
                // assert(len1 > 0);
                // assert(len2 > 0);
                // assert(base1 + len1 == base2);

                if (len1 == 1)
                {
                    return rotateLeft(base1, next(base2, len2));
                }
                if (len2 == 1)
                {
                    return rotateRight(base1, next(base2, len2));
                }

                copy_to_tmp(base2, len2);

                iter_t cursor1 = next(base1, len1);
                tmp_iter_t cursor2 = next(tmp_.begin(), len2 - 1);
                iter_t dest = next(base2, len2 - 1);

                *dest = *(--cursor1);
                --dest;
                --len1;

                int minGallop(minGallop_);

                // outer:
                while (true)
                {
                    diff_t count1 = 0;
                    diff_t count2 = 0;

                    // The next loop is a hot path of the algorithm, so we decrement
                    // eagerly the cursor so that it always points directly to the value
                    // to compare, but we have to implement some trickier logic to make
                    // sure that it points to the next value again by the end of said loop
                    --cursor1;

                    do
                    {
                        // assert(len1 > 0);
                        // assert(len2 > 1);

                        if (compare(*cursor2, *cursor1))
                        {
                            *dest = *cursor1;
                            --dest;
                            ++count1;
                            count2 = 0;
                            if (--len1 == 0)
                            {
                                goto epilogue;
                            }
                            --cursor1;
                        }
                        else
                        {
                            *dest = *cursor2;
                            --cursor2;
                            --dest;
                            ++count2;
                            count1 = 0;
                            if (--len2 == 1)
                            {
                                ++cursor1; // See comment before the loop
                                goto epilogue;
                            }
                        }
                    } while ((count1 | count2) < minGallop);
                    ++cursor1; // See comment before the loop

                    do
                    {
                        // assert(len1 > 0);
                        // assert(len2 > 1);

                        count1 = len1 - gallopRight(*cursor2, base1, len1, len1 - 1, compare);
                        if (count1 != 0)
                        {
                            std::advance(dest, -count1);
                            std::advance(cursor1, -count1);
                            len1 -= count1;
                            std::copy_backward(cursor1, next(cursor1, count1), next(dest, 1 + count1));

                            if (len1 == 0)
                            {
                                goto epilogue;
                            }
                        }
                        *dest = *cursor2;
                        --cursor2;
                        --dest;
                        if (--len2 == 1)
                        {
                            goto epilogue;
                        }

                        count2 = len2 - gallopLeft(*next(cursor1, -1), tmp_.begin(), len2, len2 - 1, compare);
                        if (count2 != 0)
                        {
                            std::advance(dest, -count2);
                            cursor2 -= count2;
                            len2 -= count2;
                            std::copy(next(cursor2, 1), next(cursor2, 1 + count2), next(dest, 1));
                            if (len2 <= 1)
                            {
                                goto epilogue;
                            }
                        }
                        *dest = *(--cursor1);
                        --dest;
                        if (--len1 == 0)
                        {
                            goto epilogue;
                        }

                        --minGallop;
                    } while ((count1 >= MIN_GALLOP) | (count2 >= MIN_GALLOP));

                    if (minGallop < 0)
                    {
                        minGallop = 0;
                    }
                    minGallop += 2;
                } // end of "outer" loop

            epilogue: // merge what is left from either cursor1 or cursor2

                minGallop_ = (std::min)(minGallop, 1);

                if (len2 == 1)
                {
                    // assert(len1 > 0);
                    std::advance(dest, -len1);
                    std::copy_backward(next(cursor1, -len1), cursor1, next(dest, 1 + len1));
                    *dest = *cursor2;
                }
                else
                {
                    // assert(len2 != 0 && "Comparison function violates its general contract");
                    // assert(len1 == 0);
                    // assert(len2 > 1);
                    std::copy(tmp_.begin(), next(tmp_.begin(), len2), next(dest, -(len2 - 1)));
                }
            }

            void copy_to_tmp(iter_t const begin, diff_t len)
            {
                tmp_.assign(begin, next(begin, len));
            }

        public:
            static void sort(iter_t const lo, iter_t const hi, Compare compare)
            {
                // assert(lo <= hi);
                diff_t nRemaining = std::distance(lo, hi);
                if (nRemaining < 2)
                {
                    return; // nothing to do
                }

                if (nRemaining < MIN_MERGE)
                {
                    diff_t const initRunLen = countRunAndMakeAscending(lo, hi, compare);
                    _TIMSORT_LOG("initRunLen: " << initRunLen);
                    binarySort(lo, hi, next(lo, initRunLen), compare);
                    return;
                }

                TimSort ts;
                diff_t const minRun = minRunLength(nRemaining);
                iter_t cur = lo;
                do
                {
                    diff_t runLen = countRunAndMakeAscending(cur, hi, compare);

                    if (runLen < minRun)
                    {
                        diff_t const force = (std::min)(nRemaining, minRun);
                        binarySort(cur, next(cur, force), next(cur, runLen), compare);
                        runLen = force;
                    }

                    ts.pushRun(cur, runLen);
                    ts.mergeCollapse(compare);

                    std::advance(cur, runLen);
                    nRemaining -= runLen;
                } while (nRemaining != 0);

                // assert(cur == hi);
                ts.mergeForceCollapse(compare);
                // assert(ts.pending_.size() == 1);

                _TIMSORT_LOG("size: " << (hi - lo) << " tmp_.size(): " << ts.tmp_.size()
                                      << " pending_.size(): " << ts.pending_.size());
            }
        };

    } // namespace detail

    // ---------------------------------------
    // Public interface implementation
    // ---------------------------------------

    /**
     * Stably sorts a range with a comparison function and a projection function.
     */
    template <typename RandomAccessIterator, typename Compare>
    void timsort(RandomAccessIterator const first, RandomAccessIterator const last, Compare compare)
    {
        detail::TimSort<RandomAccessIterator, Compare>::sort(first, last, compare);
    }

    template <typename RandomAccessIterator>
    void timsort(RandomAccessIterator const first, RandomAccessIterator const last)
    {
        typedef typename std::iterator_traits<RandomAccessIterator>::value_type value_type;
        ft::timsort(first, last, std::less<value_type>());
    }

}
