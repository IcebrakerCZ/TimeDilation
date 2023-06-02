#ifndef TIMEDILATION_TIMESPEC_H
#define TIMEDILATION_TIMESPEC_H


#include <iostream>
#include <iomanip>

#include <time.h>


inline std::ostream& operator<<(std::ostream& output, timespec const & ts)
{
  output << ts.tv_sec << "." << std::setw(9) << std::setfill('0') << ts.tv_nsec;
  return output;
}


inline timespec& operator+=(timespec& a, timespec const & b)
{
  a.tv_sec  += b.tv_sec;
  a.tv_nsec += b.tv_nsec;

  if (a.tv_nsec >= 1000000000)
  {
    a.tv_sec  += 1;
    a.tv_nsec -= 1000000000;
  }

  return a;
}


inline timespec operator+(timespec a, timespec const & b)
{
  a += b;
  return a;
}


inline timespec& operator-=(timespec& a, timespec const & b)
{
  if (a.tv_nsec < b.tv_nsec)
  {
    a.tv_sec  -= 1;
    a.tv_nsec += 1000000000;
  }

  a.tv_sec  -= b.tv_sec;
  a.tv_nsec -= b.tv_nsec;

  return a;
}


inline timespec operator-(timespec a, timespec const & b)
{
  a -= b;
  return a;
}


template <typename T>
inline timespec& operator*=(timespec& a, T b)
{
  a.tv_sec  *= b;
  a.tv_nsec *= b;

  if (a.tv_nsec >= 1000000000)
  {
    a.tv_sec  += a.tv_nsec / 1000000000;
    a.tv_nsec %= 1000000000;
  }

  return a;
}


template <typename T>
inline timespec operator*(timespec a, T b)
{
  a *= b;
  return a;
}


template <typename T>
inline timespec& operator/=(timespec& a, T b)
{
  a.tv_nsec += (a.tv_sec % b) * 1000000000;
  a.tv_nsec /= b;
  a.tv_sec  /= b;

  if (a.tv_nsec >= 1000000000)
  {
    a.tv_sec  += a.tv_nsec / 1000000000;
    a.tv_nsec %= 1000000000;
  }

  return a;
}


template <typename T>
inline timespec operator/(timespec a, T b)
{
  a /= b;
  return a;
}


inline bool operator==(const timespec& a, timespec const & b)
{
  return (a.tv_sec == b.tv_sec) && (a.tv_nsec == b.tv_nsec);
}

inline bool operator!=(const timespec& a, timespec const & b)
{
  return (a.tv_sec != b.tv_sec) || (a.tv_nsec != b.tv_nsec);
}


inline bool operator<(const timespec& a, timespec const & b)
{
  return (a.tv_sec < b.tv_sec) || (a.tv_sec == b.tv_sec && a.tv_nsec < b.tv_nsec);
}

inline bool operator<=(const timespec& a, timespec const & b)
{
  return (a == b) || (a < b);
}


inline bool operator>(const timespec& a, timespec const & b)
{
  return (a.tv_sec > b.tv_sec) || ((a.tv_sec == b.tv_sec) && (a.tv_nsec > b.tv_nsec));
}

inline bool operator>=(const timespec& a, timespec const & b)
{
  return (a == b) || (a > b);
}


#endif // TIMEDILATION_TIMESPEC_H
