/* --------------------------------------------------------------------------------------------------------------------- */

#include <map>

#include <sys/timerfd.h>

std::map<int, std::pair<clockid_t, itimerspec>> timerfd_timers_map;

/* --------------------------------------------------------------------------------------------------------------------- */

TIMEDILATION_SYMBOL_DEFINITION(timerfd_create, int, (clockid_t clockid, int flags))
{
  int fd = original_timerfd_create(clockid, flags);

  if (timedilation && fd >= 0)
  {
    timerfd_timers_map[fd].first = clockid;
  }

  return fd;
}

/* --------------------------------------------------------------------------------------------------------------------- */

TIMEDILATION_SYMBOL_DEFINITION(timerfd_settime, int, (int fd, int flags, const itimerspec *new_value, itimerspec *old_value))
{
  itimerspec timedilation_new_value = *new_value;

  if (timedilation)
  {
    try
    {
      if (flags & TFD_TIMER_ABSTIME)
      {
        // Do not dilate time on timer cancelation.
        if (!(new_value->it_value.tv_sec == 0 && new_value->it_value.tv_nsec == 1))
        {
          clockid_t clk_id = timerfd_timers_map.at(fd).first;

          timespec now;
          clock_gettime(clk_id, &now);

          timespec original_now;
          original_clock_gettime(clk_id, &original_now);

          timedilation_new_value.it_value = original_now + ((timedilation_new_value.it_value - now) * timedilation);

          TIMEDILATION_LOG_VERBOSE("fd=" << fd << " flags=" << flags << " clk_id=" << clk_id
                                   << " now=" << now << " original_now=" << original_now
                                   << " - " << new_value->it_value << " -> " << timedilation_new_value.it_value
                                   << " (diff=" << (timedilation_new_value.it_value - now)
                                   << " initial_diff=" << new_value->it_value - now << ")");
        }
      }
      else
      {
        timedilation_new_value.it_value *= timedilation;
      }

      timedilation_new_value.it_interval *= timedilation;

      TIMEDILATION_LOG_VERBOSE("fd=" << fd << " it_interval " << new_value->it_interval
                               << " -> " << timedilation_new_value.it_interval);
    }
    catch (...)
    {
      // pass
    }
  }

  int result = original_timerfd_settime(fd, flags, &timedilation_new_value, old_value);

  if (timedilation && old_value != NULL && result >= 0)
  {
    (*old_value) = timerfd_timers_map.at(fd).second;
  }

  if (timedilation)
  {
    timerfd_timers_map.at(fd).second = *new_value;
  }

  return result;
}

/* --------------------------------------------------------------------------------------------------------------------- */

TIMEDILATION_SYMBOL_DEFINITION(timerfd_gettime, int, (int fd, itimerspec *curr_value))
{
  int result = original_timerfd_gettime(fd, curr_value);

  if (timedilation && result >= 0)
  {
    curr_value->it_value    /= timedilation;
    curr_value->it_interval /= timedilation;
  }

  return result;
}

/* --------------------------------------------------------------------------------------------------------------------- */

#include <unistd.h>

TIMEDILATION_SYMBOL_DEFINITION(close, int, (int fd))
{
  timerfd_timers_map.erase(fd);

  return original_close(fd);
}

/* --------------------------------------------------------------------------------------------------------------------- */
