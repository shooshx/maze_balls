#pragma once


#include <cmath>
#include <vector>
#include <exception>
#include <iostream>

using namespace std;

/** \file
    Declares various classes and types which are used widely across the code base.
*/



typedef unsigned int uint;
typedef unsigned char byte;
typedef unsigned char uchar;
typedef unsigned short ushort;


/// takes a number and transforms it to a quoted string such as 1,234,567.09.
/// which is much more readable for humans.
string humanCount(int64_t n);
/// \overload
/// takes a float number.
string humanCount(double n, int pers);

/// flush and burn all qt events.
/// use this when visuals need to update in mid-function before moving on.
void flushAllEvents(); 

/// boolean xor
inline bool hXor(bool a, bool b)
{
    return ((a && !b) || (!a && b));
}
/// bitwise xor
inline uint bXor(uint a, uint b)
{
    return (a ^ b);
}

/// used by several different files which don't include each other. no better place to declare this.
typedef vector<int> TTransformVec;

#define DISALLOW_COPY(className) \
     className(const className &) = delete; \
     className &operator=(const className &) = delete;



template <typename T>
inline const T &mMin(const T &a, const T &b) { if (a < b) return a; return b; }
template <typename T>
inline const T &mMax(const T &a, const T &b) { if (a < b) return b; return a; }

inline float mRound(float x) {
    return floor(x + 0.5);
}


#ifdef _DEBUG
#define M_ASSERT(cond) do { if (!(cond)) throw std::runtime_error(#cond); } while(0)
#else
#define M_ASSERT(cond)
#endif

#define M_CHECK(cond) do { if (!(cond)) throw std::runtime_error(#cond); } while(0)

#define LOG_ERR(strm) do { std::cerr << strm << std::endl; } while(0)


