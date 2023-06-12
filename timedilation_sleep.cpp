#include "timespec.h"

#include <time.h>
#include <unistd.h>


TIMEDILATION_SYMBOL_DEFINITION(sleep, unsigned int, (unsigned int seconds))
{
  if (timedilation)
  {
    seconds *= timedilation;
  }

  return original_sleep(seconds);
}


TIMEDILATION_SYMBOL_DEFINITION(usleep, int, (useconds_t usec))
{
  if (timedilation)
  {
    usec *= timedilation;
  }

  return original_usleep(usec);
}


TIMEDILATION_SYMBOL_DEFINITION(nanosleep, int, (const timespec *r, timespec *rem))
{
  timespec req = *r;

  if (timedilation)
  {
    req *= timedilation;
  }

  int result = original_nanosleep(&req, rem);

  if (timedilation && rem != nullptr)
  {
    (*rem) /= timedilation;
  }

  return result;
}
