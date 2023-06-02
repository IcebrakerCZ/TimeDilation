/* --------------------------------------------------------------------------------------------------------------------- */

#include "timespec.h"
#include "timeval.h"

/* --------------------------------------------------------------------------------------------------------------------- */

extern "C" {

#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

// Time functions
typedef int (*clock_gettime_signature)(clockid_t clk_id, timespec *tp);
static clock_gettime_signature  original_clock_gettime = NULL;

static timespec  initial_clock_gettime[16];


typedef int (*gettimeofday_signature)(timeval *tp, void *tzp);
static gettimeofday_signature  original_gettimeofday = NULL;

static timeval initial_gettimeofday;


typedef time_t (*time_signature)(time_t *t);
static time_signature  original_time = NULL;

static time_t  initial_time;

// Timer functions
typedef int (*timer_create_signature)(clockid_t clockid, sigevent *sevp, timer_t *tid);
static timer_create_signature original_timer_create = NULL;

typedef int (*timer_settime_signature)(timer_t tid, int flags, const itimerspec *new_value, itimerspec *old_value);
static timer_settime_signature original_timer_settime = NULL;

typedef int (*timer_gettime_signature)(timer_t tid, itimerspec *curr_value);
static timer_gettime_signature original_timer_gettime = NULL;

typedef int (*timer_delete_signature)(timer_t tid);
static timer_delete_signature original_timer_delete = NULL;

// Timerfd functions
typedef int (*timerfd_create_signature)(clockid_t clockid, int flags);
static timerfd_create_signature original_timerfd_create = NULL;

typedef int (*timerfd_settime_signature)(int fd, int flags, const itimerspec *new_value, itimerspec *old_value);
static timerfd_settime_signature original_timerfd_settime = NULL;

typedef int (*timerfd_gettime_signature)(int fd, itimerspec *curr_value);
static timerfd_gettime_signature original_timerfd_gettime = NULL;

// Helper functions
typedef int (*close_signature)(int fd);
static close_signature original_close = NULL;

// Conditions
typedef int (*pthread_cond_init_signature)(pthread_cond_t *cond, const pthread_condattr_t *cond_attr);
static pthread_cond_init_signature original_pthread_cond_init = NULL;

typedef int (*pthread_cond_clockwait_signature)(pthread_cond_t *cond, pthread_mutex_t *mutex, clockid_t clk_id, const timespec *abstime);
static pthread_cond_clockwait_signature original_pthread_cond_clockwait = NULL;

typedef int (*pthread_cond_timedwait_signature)(pthread_cond_t *cond, pthread_mutex_t *mutex, const timespec *abstime);
static pthread_cond_timedwait_signature original_pthread_cond_timedwait = NULL;

typedef int (*pthread_cond_destroy_signature)(pthread_cond_t *cond);
static pthread_cond_destroy_signature original_pthread_cond_destroy = NULL;

// Select
#include <sys/select.h>

typedef int (*select_signature)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, timeval *timeout);
static select_signature original_select = NULL;

typedef int (*pselect_signature)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const timespec *timeout, const __sigset_t *sigmask);
static pselect_signature original_pselect = NULL;

// Poll
#include <sys/poll.h>

typedef int (*poll_signature)(pollfd *fds, nfds_t nfds, int timeout);
static poll_signature original_poll = NULL;

typedef int (*ppoll_signature)(pollfd *fds, nfds_t nfds, timespec *timeout, __sigset_t *ss);
static ppoll_signature original_ppoll = NULL;

// EPoll
#include <sys/epoll.h>

typedef int (*epoll_wait_signature)(int epfd, epoll_event *events, int maxevents, int timeout);
static epoll_wait_signature original_epoll_wait = NULL;

typedef int (*epoll_pwait_signature)(int epfd, epoll_event *events, int maxevents, int timeout, const __sigset_t *ss);
static epoll_pwait_signature original_epoll_pwait = NULL;

// Sleep
#include <time.h>
#include <unistd.h>

typedef unsigned int (*sleep_signature)(unsigned int seconds);
static sleep_signature original_sleep = NULL;

typedef int (*usleep_signature)(useconds_t usec);
static usleep_signature original_usleep = NULL;

typedef int (*nanosleep_signature)(const timespec *req, timespec *rem);
static nanosleep_signature original_nanosleep = NULL;

} // extern "C"

/* --------------------------------------------------------------------------------------------------------------------- */

extern "C" {

void timedilation_init()  __attribute__((constructor));
void timedilation_finish() __attribute__((destructor));

} // extern "C"


#include <stdio.h>
#include <unistd.h>

template<typename T>
void get_set_initial_value(const char* name, T& value)
{
  if (getenv(name) != NULL)
  {
    std::stringstream input;
    input << getenv(name);
    input >> value;

//    std::stringstream message;
//    message << getpid() << " - get_set_initial_value " << name << ": loaded " << value << " from '" << getenv(name) << "'" << std::endl;
//    printf("%s", message.str().c_str());

    return;
  }

  std::stringstream output;
  output << value;
  setenv(name, output.str().c_str(), /* overwrite = */ 0);

//  std::stringstream message;
//  message << getpid() << " - get_set_initial_value " << name << ": saved " << value << " as '" << getenv(name) << "'" << std::endl;
//  printf("%s", message.str().c_str());
}


#include <stdio.h>
#include <dlfcn.h>

#include <sstream>


static unsigned long timedilation = 1;


template<typename T>
void set_rtld_next_symbol(T& t, const char* name)
{
  const char* glibc_versions[] = { "GLIBC_2.30"
                                 , "GLIBC_2.29"
                                 , "GLIBC_2.28"
                                 , "GLIBC_2.27"
                                 , "GLIBC_2.26"
                                 , "GLIBC_2.25"
                                 , "GLIBC_2.24"
                                 , "GLIBC_2.23"
                                 , "GLIBC_2.22"
                                 , "GLIBC_2.18"
                                 , "GLIBC_2.17"
                                 , "GLIBC_2.16"
                                 , "GLIBC_2.15"
                                 , "GLIBC_2.14"
                                 , "GLIBC_2.13"
                                 , "GLIBC_2.12"
                                 , "GLIBC_2.11"
                                 , "GLIBC_2.10"
                                 , "GLIBC_2.9"
                                 , "GLIBC_2.8"
                                 , "GLIBC_2.7"
                                 , "GLIBC_2.6"
                                 , "GLIBC_2.5"
                                 , "GLIBC_2.4"
                                 , "GLIBC_2.3.4"
                                 , "GLIBC_2.3.3"
                                 , "GLIBC_2.3.2"
                                 , "GLIBC_2.17"
                                 , "GLIBC_2.16"
                                 , "GLIBC_2.15"
                                 , "GLIBC_2.14"
                                 , "GLIBC_2.13"
                                 , "GLIBC_2.12"
                                 , "GLIBC_2.11"
                                 , "GLIBC_2.10"
                                 , "GLIBC_2.9"
                                 , "GLIBC_2.8"
                                 , "GLIBC_2.7"
                                 , "GLIBC_2.6"
                                 , "GLIBC_2.5"
                                 , "GLIBC_2.4"
                                 , "GLIBC_2.3.4"
                                 , "GLIBC_2.3.3"
                                 , "GLIBC_2.3.2"
                                 , "GLIBC_2.3"
                                 , "GLIBC_2.2.6"
                                 , "GLIBC_2.2.5"
                                 };

  for (const char* glibc_version : glibc_versions)
  {
    t = (T) dlvsym(RTLD_NEXT, name, glibc_version);
    if (t != NULL)
    {
//      printf("timedilation: set_rtld_next_symbol: success %s (%s)\n", name, glibc_version);
      return;
    }
  }

  t = (T) dlsym(RTLD_NEXT, name);
  if (t != NULL)
  {
//    printf("timedilation: set_rtld_next_symbol: success %s\n", name);
    return;
  }

//  printf("timedilation: set_rtld_next_symbol: failure '%s'\n", name);
}

void timedilation_init()
{
//  printf("timedilation_init: begin\n");

  set_rtld_next_symbol(original_clock_gettime, "clock_gettime");
  set_rtld_next_symbol(original_gettimeofday , "gettimeofday");
  set_rtld_next_symbol(original_time         , "time");

  set_rtld_next_symbol(original_timer_create , "timer_create");
  set_rtld_next_symbol(original_timer_settime, "timer_settime");
  set_rtld_next_symbol(original_timer_gettime, "timer_gettime");
  set_rtld_next_symbol(original_timer_delete , "timer_delete");

  set_rtld_next_symbol(original_timerfd_create , "timerfd_create");
  set_rtld_next_symbol(original_timerfd_settime, "timerfd_settime");
  set_rtld_next_symbol(original_timerfd_gettime, "timerfd_gettime");

  set_rtld_next_symbol(original_close, "close");

  set_rtld_next_symbol(original_pthread_cond_init     , "pthread_cond_init");
  set_rtld_next_symbol(original_pthread_cond_clockwait, "pthread_cond_clockwait");
  set_rtld_next_symbol(original_pthread_cond_timedwait, "pthread_cond_timedwait");
  set_rtld_next_symbol(original_pthread_cond_destroy  , "pthread_cond_destroy");

  set_rtld_next_symbol(original_select , "select");
  set_rtld_next_symbol(original_pselect, "pselect");

  set_rtld_next_symbol(original_poll , "poll");
  set_rtld_next_symbol(original_ppoll, "ppoll");

  set_rtld_next_symbol(original_epoll_wait , "epoll_wait");
  set_rtld_next_symbol(original_epoll_pwait, "epoll_pwait");

  set_rtld_next_symbol(original_sleep, "sleep");
  set_rtld_next_symbol(original_sleep, "usleep");
  set_rtld_next_symbol(original_sleep, "nanosleep");

  if (getenv("TIMEDILATION") == NULL)
  {
//    printf("timedilation_init: no timedilation environment variable found\n");
  }
  else
  {
    std::stringstream input;
    input << getenv("TIMEDILATION");
    input >> timedilation;
//    printf("timedilation_init: found timedilation environment variable: '%s' -> %lu\n", getenv("timedilation"), timedilation);
  }

//  printf("timedilation_init: timedilation=%lu\n", timedilation);


  for (clockid_t clk_id = 0; clk_id < static_cast<clockid_t>(sizeof(initial_clock_gettime) / sizeof(timespec)); ++clk_id)
  {
    original_clock_gettime(clk_id, &initial_clock_gettime[clk_id]);
    std::stringstream clock_name;
    clock_name << "TIMEDILATION_INITIAL_CLOCK_GETTIME_" << clk_id;
    get_set_initial_value((clock_name.str() + "_TV_SEC").c_str() , initial_clock_gettime[clk_id].tv_sec);
    get_set_initial_value((clock_name.str() + "_TV_NSEC").c_str(), initial_clock_gettime[clk_id].tv_nsec);
  }

  original_gettimeofday(&initial_gettimeofday, NULL);
  get_set_initial_value("TIMEDILATION_INITIAL_GETTIMEOFDAY_TV_SEC" , initial_gettimeofday.tv_sec);
  get_set_initial_value("TIMEDILATION_INITIAL_GETTIMEOFDAY_TV_USEC", initial_gettimeofday.tv_usec);

  initial_time = original_time(NULL);
  get_set_initial_value("TIMEDILATION_INITIAL_TIME", initial_time);

//  printf("timedilation_init: end\n");
}


void timedilation_finish()
{
//  printf("timedilation_finish: begin\n");
//  printf("timedilation_finish: end\n");
}

/* --------------------------------------------------------------------------------------------------------------------- */

#include <time.h>

//thread_local bool debug_clock_gettime = false;

int clock_gettime(clockid_t clk_id, timespec *tp)
{
  int status = original_clock_gettime(clk_id, tp);
//  timespec original_now = *tp;

  if (timedilation && status >= 0)
  {
    (*tp) = initial_clock_gettime[clk_id] + (((*tp) - initial_clock_gettime[clk_id]) / timedilation);

//    if (debug_clock_gettime)
//    {
//      std::stringstream message;
//      message << "timedilation: clock_gettime: clk_id=" << clk_id << " initial_clock_gettime[clk_id]=" << initial_clock_gettime[clk_id] << " tp=" << *tp << " original_now=" << original_now << std::endl;
//      printf("%s", message.str().c_str());
//    }
  }

  return status;
}

/* --------------------------------------------------------------------------------------------------------------------- */

#include <sys/time.h>

int gettimeofday(timeval *tp, void *tzp)
{
  int status = original_gettimeofday(tp, tzp);
//  timeval original_now = *tp;

  if (timedilation && status >= 0)
  {
    (*tp) = initial_gettimeofday + (((*tp) - initial_gettimeofday) / timedilation);
  }

//  std::stringstream message;
//  message << "timedilation: gettimeofday: tp=" << *tp << " original_now=" << original_now << std::endl;
//  printf("%s", message.str().c_str());

  return status;
}

/* --------------------------------------------------------------------------------------------------------------------- */

#include <time.h>

time_t time(time_t *t)
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

/* --------------------------------------------------------------------------------------------------------------------- */

#include <map>

#include <sys/timerfd.h>

std::map<int, std::pair<clockid_t, itimerspec>> timerfd_timers_map;

int timerfd_create(clockid_t clockid, int flags)
{
  int fd = original_timerfd_create(clockid, flags);

  if (timedilation && fd >= 0)
  {
    timerfd_timers_map[fd].first = clockid;
  }

  return fd;
}

int timerfd_settime(int fd, int flags, const itimerspec *new_value, itimerspec *old_value)
{
  itimerspec timedilation_new_value = *new_value;

  if (timedilation)
  {
    try
    {
      if (flags & TFD_TIMER_ABSTIME)
      {
        // Do not time dilate timer cancelation.
        if (!(new_value->it_value.tv_sec == 0 && new_value->it_value.tv_nsec == 1))
        {
          clockid_t clk_id = timerfd_timers_map.at(fd).first;

          timespec now;
          clock_gettime(clk_id, &now);

          timespec original_now;
          original_clock_gettime(clk_id, &original_now);

          timedilation_new_value.it_value = original_now + ((timedilation_new_value.it_value - now) * timedilation);

//          std::stringstream message;
//          message << "timedilation: timerfd_settime: fd=" << fd << " flags=" << flags << " clk_id=" << clk_id
//                  << " now=" << now << " original_now=" << original_now
//                  << " - " << new_value->it_value << " -> " << timedilation_new_value.it_value
//                  << " (diff=" << (timedilation_new_value.it_value - now)
//                  << " initial_diff=" << new_value->it_value - now << ")"
//                  << std::endl;
//          printf("%s", message.str().c_str());
        }
      }
      else
      {
        timedilation_new_value.it_value *= timedilation;
      }

      timedilation_new_value.it_interval *= timedilation;
//    std::stringstream message;
//    message << "timedilation: timerfd_settime: fd=" << fd << " it_interval "
//            << new_value->it_interval << " -> " << timedilation_new_value.it_interval
//            << std::endl;
//    printf("%s", message.str().c_str());
    }
    catch (...)
    {
//      printf("timedilation: timerfd_settime: fd=%i not in map\n", fd);
//      abort();
      // pass
    }
  }

  int result = original_timerfd_settime(fd, flags, &timedilation_new_value, old_value);

  if (timedilation && old_value != NULL && result >= 0)
  {
    (*old_value) = timerfd_timers_map.at(fd).second;
//    old_value->it_value    /= timedilation;
//    old_value->it_interval /= timedilation;
  }

  timerfd_timers_map.at(fd).second = *new_value;

  return result;
}

int timerfd_gettime(int fd, itimerspec *curr_value)
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

int close(int fd)
{
  timerfd_timers_map.erase(fd);

  return original_close(fd);
}

/* --------------------------------------------------------------------------------------------------------------------- */

#include <map>

#include <signal.h>
#include <time.h>

std::map<timer_t, std::pair<clockid_t, itimerspec>> timer_timers_map;

int timer_create(clockid_t clockid, sigevent *sevp, timer_t *tid)
{
  int result = original_timer_create(clockid, sevp, tid);

  if (timedilation && result >= 0)
  {
    timer_timers_map[*tid].first = clockid;
  }

  return result;
}

int timer_settime(timer_t tid, int flags, const itimerspec *new_value, itimerspec *old_value)
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

//          std::stringstream message;
//          message << "timedilation: timer_settime: tid=" << (void*) &tid << " clk_id=" << clk_id << " now=" << now << " original_now=" << original_now << " - " << new_value->it_value << " -> " << timedilation_new_value.it_value << " (diff=" << (timedilation_new_value.it_value - now) << " initial_diff=" << new_value->it_value - now << ")" << std::endl;
//          printf("%s", message.str().c_str());
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
      // pass
    }
  }

  int result = original_timer_settime(tid, flags, &timedilation_new_value, old_value);

  if (timedilation && old_value != NULL && result >= 0)
  {
    (*old_value) = timer_timers_map.at(tid).second;
//    old_value->it_value    /= timedilation;
//    old_value->it_interval /= timedilation;
  }

  timer_timers_map.at(tid).second = *new_value;

  return result;
}

int timer_gettime(timer_t tid, itimerspec *curr_value)
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

int timer_delete(timer_t tid)
{
  timer_timers_map.erase(tid);

  return original_timer_delete(tid);
}

/* --------------------------------------------------------------------------------------------------------------------- */

#include <map>

std::map<pthread_cond_t*, clockid_t> pthread_cond_clock_ids_map;

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *cond_attr)
{
  int result = original_pthread_cond_init(cond, cond_attr);

  if (timedilation && result >= 0)
  {
    clockid_t clk_id = CLOCK_REALTIME;

    if (cond_attr != NULL)
    {
      pthread_condattr_getclock(cond_attr, &clk_id);
    }

    pthread_cond_clock_ids_map[cond] = clk_id;
  }

  return result;
}

/* --------------------------------------------------------------------------------------------------------------------- */

int pthread_cond_clockwait(pthread_cond_t *cond, pthread_mutex_t *mutex, clockid_t clk_id, const timespec *abstime)
{
  timespec newtime = *abstime;

  if (timedilation)
  {
//    debug_clock_gettime = true;
    timespec now;
    clock_gettime(clk_id, &now);

    timespec original_now;
    original_clock_gettime(clk_id, &original_now);

//    debug_clock_gettime = false;

    newtime = original_now + (((*abstime) - now) * timedilation);

//    std::stringstream message;
//    message << "timedilation: pthread_cond_clockwait: cond=" << (void*) cond << " clk_id=" << clk_id << " now=" << now << " - " << *abstime << " -> " << newtime << " (diff=" << (newtime - now) << " initial_diff=" << (*abstime) - now << ")" << std::endl;
//    printf("%s", message.str().c_str());
  }

  return original_pthread_cond_clockwait(cond, mutex, clk_id, &newtime);
}

/* --------------------------------------------------------------------------------------------------------------------- */

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const timespec *abstime)
{
  clockid_t clk_id = CLOCK_REALTIME;

  if (timedilation)
  {
    try
    {
      clk_id = pthread_cond_clock_ids_map.at(cond);
    }
    catch (...)
    {
      // pass
//      printf("timedilation: pthread_cond_timedwait: cond=%p not found in the pthread_cond_clock_ids_map\n", cond);
    }
  }

//  std::stringstream message;
//  message << "timedilation: pthread_cond_timedwait: cond=" << (void*) cond << " clk_id=" << clk_id << " abstime=" << *abstime << std::endl;
//  printf("%s", message.str().c_str());

  return pthread_cond_clockwait(cond, mutex, clk_id, abstime);
}

/* --------------------------------------------------------------------------------------------------------------------- */

int pthread_cond_destroy(pthread_cond_t *cond)
{
  pthread_cond_clock_ids_map.erase(cond);

  return original_pthread_cond_destroy(cond);
}

/* --------------------------------------------------------------------------------------------------------------------- */

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, timeval *timeout)
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

/* --------------------------------------------------------------------------------------------------------------------- */

int pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const timespec *timeout, const __sigset_t *sigmask)
{
  if (!timedilation || timeout == NULL)
  {
    return original_pselect(nfds, readfds, writefds, exceptfds, timeout, sigmask);
  }

  timespec newtimeout = (*timeout) * timedilation;

  return original_pselect(nfds, readfds, writefds, exceptfds, &newtimeout, sigmask);
}

/* --------------------------------------------------------------------------------------------------------------------- */

int poll(pollfd *fds, nfds_t nfds, int timeout)
{
  if (timedilation && timeout > 0)
  {
    timeout *= timedilation;
  }

  return original_poll(fds, nfds, timeout);
}

/* --------------------------------------------------------------------------------------------------------------------- */

int ppoll(pollfd *fds, nfds_t nfds, timespec *timeout, __sigset_t *ss)
{
  if (!timedilation || timeout == NULL)
  {
    return original_ppoll(fds, nfds, timeout, ss);
  }

  timespec newtimeout = (*timeout) * timedilation;

  return original_ppoll(fds, nfds, &newtimeout, ss);
}

/* --------------------------------------------------------------------------------------------------------------------- */

int epoll_wait(int epfd, epoll_event *events, int maxevents, int timeout)
{
  if (timedilation && timeout > 0)
  {
    timeout *= timedilation;
  }

  return original_epoll_wait(epfd, events, maxevents, timeout);
}

/* --------------------------------------------------------------------------------------------------------------------- */

int epoll_pwait(int epfd, epoll_event *events, int maxevents, int timeout, const __sigset_t *ss)
{
  if (timedilation && timeout > 0)
  {
    timeout *= timedilation;
  }

  return original_epoll_pwait(epfd, events, maxevents, timeout, ss);
}

/* --------------------------------------------------------------------------------------------------------------------- */

unsigned int sleep(unsigned int seconds)
{
  if (timedilation)
  {
    seconds *= timedilation;
  }

  return original_sleep(seconds);
}

/* --------------------------------------------------------------------------------------------------------------------- */

int usleep(useconds_t usec)
{
  if (timedilation)
  {
    usec *= timedilation;
  }

  return original_usleep(usec);
}

/* --------------------------------------------------------------------------------------------------------------------- */

int nanosleep(const timespec *r, timespec *rem)
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

/* --------------------------------------------------------------------------------------------------------------------- */
