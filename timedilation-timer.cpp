/* --------------------------------------------------------------------------------------------------------------------- */

#include <map>

#include <signal.h>
#include <time.h>

std::map<timer_t, std::pair<clockid_t, itimerspec>> timer_timers_map;

/* --------------------------------------------------------------------------------------------------------------------- */

TIMEDILATION_SYMBOL_DEFINITION(timer_create, int, (clockid_t clockid, sigevent *sevp, timer_t *tid))
{
  int result = original_timer_create(clockid, sevp, tid);

  if (timedilation && result >= 0)
  {
    timer_timers_map[*tid].first = clockid;
  }

  return result;
}

/* --------------------------------------------------------------------------------------------------------------------- */

TIMEDILATION_SYMBOL_DEFINITION(timer_settime, int, (timer_t tid, int flags, const itimerspec *new_value, itimerspec *old_value))
{
  itimerspec timedilation_new_value = *new_value;

  if (timedilation)
  {
    try
    {
      if (flags & TIMER_ABSTIME)
      {
        // Do not timedilation timer cancelation.
        if (!(new_value->it_value.tv_sec == 0 && new_value->it_value.tv_nsec == 1))
        {
          clockid_t clk_id = timer_timers_map.at(tid).first;

          timespec now;
          clock_gettime(clk_id, &now);

          timespec original_now;
          original_clock_gettime(clk_id, &original_now);

          timedilation_new_value.it_value = original_now + ((timedilation_new_value.it_value - now) * timedilation);

          TIMEDILATION_LOG_VERBOSE("tid=" << (void*) &tid << " clk_id=" << clk_id << " now=" << now << " original_now="
                                   << original_now << " - " << new_value->it_value << " -> " << timedilation_new_value.it_value
                                   << " (diff=" << (timedilation_new_value.it_value - now) << " initial_diff="
                                   << new_value->it_value - now << ")");
        }
      }
      else
      {
        timedilation_new_value.it_value *= timedilation;
      }

      timedilation_new_value.it_interval *= timedilation;
    }
    catch (...)
    {
      printf("timedilation: timer_settime: tid=%p not in map\n", &tid);
      abort();
    }
  }

  int result = original_timer_settime(tid, flags, &timedilation_new_value, old_value);

  if (timedilation && old_value != NULL && result >= 0)
  {
    (*old_value) = timer_timers_map.at(tid).second;
  }

  timer_timers_map.at(tid).second = *new_value;

  return result;
}

/* --------------------------------------------------------------------------------------------------------------------- */

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

/* --------------------------------------------------------------------------------------------------------------------- */

TIMEDILATION_SYMBOL_DEFINITION(timer_delete, int, (timer_t tid))
{
  timer_timers_map.erase(tid);

  return original_timer_delete(tid);
}

/* --------------------------------------------------------------------------------------------------------------------- */
