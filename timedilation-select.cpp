#include "timeval.h"

#include <sys/select.h>


TIMEDILATION_SYMBOL_DEFINITION(select, int, (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, timeval *timeout))
{
  if (!timedilation || timeout == NULL)
  {
    return original_select(nfds, readfds, writefds, exceptfds, timeout);
  }

  timeval newtimeout = (*timeout) * timedilation;
  timeval original_newtimeout = newtimeout;

  int status = original_select(nfds, readfds, writefds, exceptfds, &newtimeout);

  if (timeout != NULL && newtimeout != original_newtimeout)
  {
    (*timeout) = newtimeout / timedilation;
  }

  return status;
}


TIMEDILATION_SYMBOL_DEFINITION(pselect, int, (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const timespec *timeout, const __sigset_t *sigmask))
{
  if (!timedilation || timeout == NULL)
  {
    return original_pselect(nfds, readfds, writefds, exceptfds, timeout, sigmask);
  }

  timespec newtimeout = (*timeout) * timedilation;

  return original_pselect(nfds, readfds, writefds, exceptfds, &newtimeout, sigmask);
}
