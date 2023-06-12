#include "timespec.h"

#include <map>

#include <signal.h>
#include <time.h>


std::map<timer_t, std::pair<clockid_t, itimerspec>> timer_timers_map;


TIMEDILATION_SYMBOL_DEFINITION(timer_create, int, (clockid_t clockid, sigevent *sevp, timer_t *tid))
{
  int result = original_timer_create(clockid, sevp, tid);

  if (timedilation && result >= 0)
  {
    timer_timers_map[*tid].first = clockid;
  }

  return result;
}


TIMEDILATION_SYMBOL_DEFINITION(timer_settime, int, (timer_t tid, int flags, const itimerspec *new_value, itimerspec *old_value))
{
  itimerspec timedilation_new_value = *new_value;

  if (timedilation)
  {
    try
    {
      if (flags & TIMER_ABSTIME)
      {
        if (new_value->it_value.tv_sec == 0 && new_value->it_value.tv_nsec == 1)
        {
          // Do not dilate time on timer cancel.
        }
        else
        {
          clockid_t clk_id = timer_timers_map.at(tid).first;

          timedilation_new_value.it_value = initial_clock_gettime[clk_id]
                                          + ((timedilation_new_value.it_value - initial_clock_gettime[clk_id]) * timedilation);

          TIMEDILATION_LOG_VERBOSE("tid=" << (void*) &tid << " flags=" << flags << " clk_id=" << clk_id
                                   << " initial_clock_gettime[clk_id]=" << initial_clock_gettime[clk_id]
                                   << " - it_value " << new_value->it_value << " -> " << timedilation_new_value.it_value
                                   << " (new_diff=" << (timedilation_new_value.it_value - initial_clock_gettime[clk_id])
                                   << " initial_diff=" << (new_value->it_value - initial_clock_gettime[clk_id]) << ")");
        }
      }
      else
      {
        timedilation_new_value.it_value *= timedilation;

        TIMEDILATION_LOG_VERBOSE("tid=" << (void*) &tid << " flags=" << flags
                                 << " - it_value " << new_value->it_value << " -> " << timedilation_new_value.it_value);
      }

      timedilation_new_value.it_interval *= timedilation;

      TIMEDILATION_LOG_VERBOSE("tid=" << (void*) &tid << " flags=" << flags
                               << " - it_interval " << new_value->it_interval
                               << " -> " << timedilation_new_value.it_interval);
    }
    catch (...)
    {
      TIMEDILATION_LOG_VERBOSE("tid=" << (void*) &tid << " not in map");
      // pass
    }
  }

  int result = original_timer_settime(tid, flags, &timedilation_new_value, old_value);

  if (timedilation && result >= 0)
  {
    if (old_value != NULL)
    {
      (*old_value) = timer_timers_map.at(tid).second;
    }

    timer_timers_map.at(tid).second = *new_value;
  }

  return result;
}


TIMEDILATION_SYMBOL_DEFINITION(timer_gettime, int, (timer_t tid, itimerspec *curr_value))
{
  int result = original_timer_gettime(tid, curr_value);

  if (timedilation && result >= 0)
  {
    curr_value->it_value    /= timedilation;
    curr_value->it_interval /= timedilation;
  }

  return result;
}


TIMEDILATION_SYMBOL_DEFINITION(timer_delete, int, (timer_t tid))
{
  timer_timers_map.erase(tid);

  return original_timer_delete(tid);
}
