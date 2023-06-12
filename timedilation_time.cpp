#include "timespec.h"

#include <sys/time.h>
#include <time.h>


TIMEDILATION_SYMBOL_DEFINITION(clock_gettime, int, (clockid_t clk_id, timespec *tp))
{
  int status = original_clock_gettime(clk_id, tp);

  if (timedilation && status >= 0)
  {
    (*tp) = initial_clock_gettime[clk_id] + (((*tp) - initial_clock_gettime[clk_id]) / timedilation);
  }

  return status;
}


TIMEDILATION_SYMBOL_DEFINITION(gettimeofday, int, (timeval *tp, void *tzp))
{
  int status = original_gettimeofday(tp, tzp);

  if (timedilation && status >= 0)
  {
    (*tp) = initial_gettimeofday + (((*tp) - initial_gettimeofday) / timedilation);
  }

  return status;
}


TIMEDILATION_SYMBOL_DEFINITION(time, time_t, (time_t *t))
{
  time_t tv = original_time(t);

  if (timedilation && tv != ((time_t) -1))
  {
    if (t != NULL)
    {
       (*t) = initial_time + (((*t) - initial_time) / timedilation);
    }

    tv = initial_time + ((tv - initial_time) / timedilation);
  }

  return tv;
}
