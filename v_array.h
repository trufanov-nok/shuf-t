/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE_VW.
 */

#pragma once
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>


#ifdef _WIN32
#define __INLINE
#else
#define __INLINE inline
#endif

const size_t erase_point = ~ ((1 << 10) -1);

template<class T> struct v_array
{
public:
    T* begin;
    T* end;
    T* end_array;
    size_t erase_count;

    T last() { return *(end-1);}
    T pop() { return *(--end);}
    bool empty() { return begin == end;}
    void decr() { end--;}
    void incr()
    { if (end == end_array)
            resize(2 * (end_array - begin) + 3);
        end++;
    }
    T& operator[](size_t i) { return begin[i]; }
    T& get(size_t i) { return begin[i]; }
    inline size_t size() {return end-begin;}
    void resize(size_t length, bool zero_everything=false)
    { if ((size_t)(end_array-begin) != length)
        { size_t old_len = end-begin;
            T* temp = (T *)realloc(begin, sizeof(T) * length);
            if ((temp == NULL) && ((sizeof(T)*length) > 0))
            {
                std::cerr << "realloc of " << length << " failed in resize().  out of memory?";
                throw -1;
            }
            else
                begin = temp;
            if (zero_everything && (old_len < length))
                memset(begin+old_len, 0, (length-old_len)*sizeof(T));
            end = begin+old_len;
            end_array = begin + length;
        }
    }

    void erase()
    { if (++erase_count & erase_point)
        { resize(end-begin);
            erase_count = 0;
        }
        end = begin;
    }
    void delete_v()
    { if (begin != NULL)
            free(begin);
        begin = end = end_array = NULL;
    }
    void push_back(const T &new_ele)
    { if(end == end_array)
            resize(2 * (end_array-begin) + 3);
        *(end++) = new_ele;
    }

};
