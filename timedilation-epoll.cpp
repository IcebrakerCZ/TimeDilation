/* --------------------------------------------------------------------------------------------------------------------- */

#include <sys/epoll.h>

/* --------------------------------------------------------------------------------------------------------------------- */

TIMEDILATION_SYMBOL_DEFINITION(epoll_wait, int, (int epfd, epoll_event *events, int maxevents, int timeout))
{
  if (timedilation && timeout > 0)
  {
    timeout *= timedilation;
  }

  return original_epoll_wait(epfd, events, maxevents, timeout);
}

/* --------------------------------------------------------------------------------------------------------------------- */

TIMEDILATION_SYMBOL_DEFINITION(epoll_pwait, int, (int epfd, epoll_event *events, int maxevents, int timeout, const __sigset_t *ss))
{
  if (timedilation && timeout > 0)
  {
    timeout *= timedilation;
  }

  return original_epoll_pwait(epfd, events, maxevents, timeout, ss);
}

/* --------------------------------------------------------------------------------------------------------------------- */
