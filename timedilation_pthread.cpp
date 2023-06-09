#include "timespec.h"


TIMEDILATION_SYMBOL_DEFINITION(pthread_cond_clockwait, int, (pthread_cond_t *cond, pthread_mutex_t *mutex, clockid_t clk_id, const timespec *abstime))
{
  timespec newabstime = *abstime;

  if (timedilation)
  {
    newabstime = initial_clock_gettime[clk_id] + (((*abstime) - initial_clock_gettime[clk_id]) * timedilation);
  }

  return original_pthread_cond_clockwait(cond, mutex, clk_id, &newabstime);
}


TIMEDILATION_SYMBOL_DEFINITION(pthread_cond_timedwait, int, (pthread_cond_t *cond, pthread_mutex_t *mutex, const timespec *abstime))
{
  timespec newabstime = *abstime;

  if (timedilation)
  {
    timespec initial_gettimeofday_timespec = { .tv_sec = initial_gettimeofday.tv_sec,
                                               .tv_nsec = initial_gettimeofday.tv_usec * 1000 };

    newabstime = initial_gettimeofday_timespec + (((*abstime) - initial_gettimeofday_timespec) * timedilation);
  }

  return original_pthread_cond_timedwait(cond, mutex, &newabstime);
}


TIMEDILATION_SYMBOL_DEFINITION(pthread_mutex_clocklock, int, (pthread_mutex_t *mutex, clockid_t clk_id, const timespec *abstime))
{
  timespec newabstime = *abstime;

  if (timedilation)
  {
    newabstime = initial_clock_gettime[clk_id] + (((*abstime) - initial_clock_gettime[clk_id]) * timedilation);
  }

  return original_pthread_mutex_clocklock(mutex, clk_id, &newabstime);
}


TIMEDILATION_SYMBOL_DEFINITION(pthread_mutex_timedlock, int, (pthread_mutex_t *mutex, const timespec *abstime))
{
  timespec newabstime = *abstime;

  if (timedilation)
  {
    newabstime = initial_clock_gettime[CLOCK_REALTIME] + (((*abstime) - initial_clock_gettime[CLOCK_REALTIME]) * timedilation);
  }

  return original_pthread_mutex_timedlock(mutex, &newabstime);
}


TIMEDILATION_SYMBOL_DEFINITION(pthread_rwlock_clockrdlock, int, (pthread_mutex_t *mutex, clockid_t clk_id, const timespec *abstime))
{
  timespec newabstime = *abstime;

  if (timedilation)
  {
    newabstime = initial_clock_gettime[clk_id] + (((*abstime) - initial_clock_gettime[clk_id]) * timedilation);
  }

  return original_pthread_rwlock_clockrdlock(mutex, clk_id, &newabstime);
}


TIMEDILATION_SYMBOL_DEFINITION(pthread_rwlock_timedrdlock, int, (pthread_mutex_t *mutex, const timespec *abstime))
{
  timespec newabstime = *abstime;

  if (timedilation)
  {
    newabstime = initial_clock_gettime[CLOCK_REALTIME] + (((*abstime) - initial_clock_gettime[CLOCK_REALTIME]) * timedilation);
  }

  return original_pthread_rwlock_timedrdlock(mutex, &newabstime);
}


TIMEDILATION_SYMBOL_DEFINITION(pthread_rwlock_clockwrlock, int, (pthread_mutex_t *mutex, clockid_t clk_id, const timespec *abstime))
{
  timespec newabstime = *abstime;

  if (timedilation)
  {
    newabstime = initial_clock_gettime[clk_id] + (((*abstime) - initial_clock_gettime[clk_id]) * timedilation);
  }

  return original_pthread_rwlock_clockwrlock(mutex, clk_id, &newabstime);
}


TIMEDILATION_SYMBOL_DEFINITION(pthread_rwlock_timedwrlock, int, (pthread_mutex_t *mutex, const timespec *abstime))
{
  timespec newabstime = *abstime;

  if (timedilation)
  {
    newabstime = initial_clock_gettime[CLOCK_REALTIME] + (((*abstime) - initial_clock_gettime[CLOCK_REALTIME]) * timedilation);
  }

  return original_pthread_rwlock_timedwrlock(mutex, &newabstime);
}
