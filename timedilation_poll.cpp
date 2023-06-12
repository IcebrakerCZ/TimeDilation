#include "timespec.h"

#include <sys/poll.h>


TIMEDILATION_SYMBOL_DEFINITION(poll, int, (pollfd *fds, nfds_t nfds, int timeout))
{
  if (timedilation && timeout > 0)
  {
    timeout *= timedilation;
  }

  return original_poll(fds, nfds, timeout);
}


TIMEDILATION_SYMBOL_DEFINITION(ppoll, int, (pollfd *fds, nfds_t nfds, timespec *timeout, __sigset_t *ss))
{
  if (!timedilation || timeout == NULL)
  {
    return original_ppoll(fds, nfds, timeout, ss);
  }

  timespec newtimeout = (*timeout) * timedilation;

  return original_ppoll(fds, nfds, &newtimeout, ss);
}
