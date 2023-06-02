#ifndef ICETRACE_TIMEVAL_H
#define ICETRACE_TIMEVAL_H


#include <iostream>
#include <iomanip>

#include <sys/time.h>


inline std::ostream& operator<<(std::ostream& output, timeval const & tv)
{
  output << tv.tv_sec << "." << std::setw(6) << std::setfill('0') << tv.tv_usec;
  return output;
}


inline timeval& operator+=(timeval& a, timeval const & b)
{
  a.tv_sec  += b.tv_sec;
  a.tv_usec += b.tv_usec;

  if (a.tv_usec >= 1000000)
  {
    a.tv_sec  += 1;
    a.tv_usec -= 1000000;
  }

  return a;
}


inline timeval operator+(timeval a, timeval const & b)
{
  a += b;
  return a;
}


inline timeval& operator-=(timeval& a, timeval const & b)
{
  if (a.tv_usec < b.tv_usec)
  {
    a.tv_sec  -= 1;
    a.tv_usec += 1000000;
  }

  a.tv_sec  -= b.tv_sec;
  a.tv_usec -= b.tv_usec;

  return a;
}


inline timeval operator-(timeval a, timeval const & b)
{
  a -= b;
  return a;
}


template <typename T>
inline timeval& operator*=(timeval& a, T b)
{
  a.tv_sec  *= b;
  a.tv_usec *= b;

  if (a.tv_usec >= 1000000)
  {
    a.tv_sec  += a.tv_usec / 1000000;
    a.tv_usec %= 1000000;
  }

  return a;
}


template <typename T>
inline timeval operator*(timeval a, T b)
{
  a *= b;
  return a;
}


template <typename T>
inline timeval& operator/=(timeval& a, T b)
{
  a.tv_usec += (a.tv_sec % b) * 1000000;
  a.tv_usec /= b;
  a.tv_sec  /= b;

  if (a.tv_usec >= 1000000)
  {
    a.tv_sec  += a.tv_usec / 1000000;
    a.tv_usec %= 1000000;
  }

  return a;
}


template <typename T>
inline timeval operator/(timeval a, T b)
{
  a /= b;
  return a;
}


inline bool operator==(const timeval& a, timeval const & b)
{
  return (a.tv_sec == b.tv_sec) && (a.tv_usec == b.tv_usec);
}

inline bool operator!=(const timeval& a, timeval const & b)
{
  return (a.tv_sec != b.tv_sec) || (a.tv_usec != b.tv_usec);
}


inline bool operator<(const timeval& a, timeval const & b)
{
  return (a.tv_sec < b.tv_sec) || (a.tv_sec == b.tv_sec && a.tv_usec < b.tv_usec);
}

inline bool operator<=(const timeval& a, timeval const & b)
{
  return (a == b) || (a < b);
}


inline bool operator>(const timeval& a, timeval const & b)
{
  return (a.tv_sec > b.tv_sec) || ((a.tv_sec == b.tv_sec) && (a.tv_usec > b.tv_usec));
}

inline bool operator>=(const timeval& a, timeval const & b)
{
  return (a == b) || (a > b);
}


#endif // ICETRACE_TIMEVAL_H
