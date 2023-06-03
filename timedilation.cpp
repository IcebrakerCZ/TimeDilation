/* --------------------------------------------------------------------------------------------------------------------- */

#include "timespec.h"
#include "timeval.h"

/* --------------------------------------------------------------------------------------------------------------------- */

static unsigned int timedilation = 0;

static bool timedilation_verbose = false;

/* --------------------------------------------------------------------------------------------------------------------- */

#define TIMEDILATION_LOG_VERBOSE(message)          \
  do {                                             \
    std::stringstream m;                           \
    m << message;                                  \
    printf("timedilation: %s\n", m.str().c_str()); \
  } while(0)


#define TIMEDILATION_SYMBOL_DEFINITION(symbol_name, return_type, input_args) \
    extern "C"                                                               \
    {                                                                        \
      typedef return_type (*symbol_name ## _signature)input_args;            \
      static symbol_name ## _signature  original_ ## symbol_name = NULL;     \
    }                                                                        \
                                                                             \
    return_type symbol_name input_args


#include <time.h>
#include <sys/time.h>

static timespec  initial_clock_gettime[16];
static timeval   initial_gettimeofday;
static time_t    initial_time;

#include "timedilation-epoll.cpp"
#include "timedilation-poll.cpp"
#include "timedilation-pthread_cond.cpp"
#include "timedilation-select.cpp"
#include "timedilation-sleep.cpp"
#include "timedilation-time.cpp"
#include "timedilation-timer.cpp"
#include "timedilation-timerfd.cpp"

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

    TIMEDILATION_LOG_VERBOSE("get_set_initial_value: " << name << ": loaded " << value << " from '" << getenv(name) << "'");

    return;
  }

  std::stringstream output;
  output << value;
  setenv(name, output.str().c_str(), /* overwrite = */ 0);

  TIMEDILATION_LOG_VERBOSE("get_set_initial_value: " << name << ": saved " << value << " as '" << getenv(name) << "'");
}


#include <stdio.h>
#include <dlfcn.h>

#include <sstream>

#include "glibc_versions.h"


template<typename T>
void set_rtld_next_symbol(T& t, const char* name)
{
  for (const char* glibc_version : glibc_versions)
  {
    t = (T) dlvsym(RTLD_NEXT, name, glibc_version);
    if (t != NULL)
    {
      TIMEDILATION_LOG_VERBOSE("success " << name << " (" << glibc_version << ")");
      return;
    }
  }

  t = (T) dlsym(RTLD_NEXT, name);
  if (t != NULL)
  {
    TIMEDILATION_LOG_VERBOSE("success " << name);
    return;
  }

  printf("timedilation: set_rtld_next_symbol: symbol '%s' not found\n", name);
  abort();
}


void timedilation_init()
{
  if (getenv("TIMEDILATION_VERBOSE"))
  {
    timedilation_verbose = true;
  }

  TIMEDILATION_LOG_VERBOSE("begin");

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

  set_rtld_next_symbol(original_pthread_cond_clockwait, "pthread_cond_clockwait");
  set_rtld_next_symbol(original_pthread_cond_timedwait, "pthread_cond_timedwait");

  set_rtld_next_symbol(original_select , "select");
  set_rtld_next_symbol(original_pselect, "pselect");

  set_rtld_next_symbol(original_poll , "poll");
  set_rtld_next_symbol(original_ppoll, "ppoll");

  set_rtld_next_symbol(original_epoll_wait , "epoll_wait");
  set_rtld_next_symbol(original_epoll_pwait, "epoll_pwait");

  set_rtld_next_symbol(original_sleep, "sleep");
  set_rtld_next_symbol(original_usleep, "usleep");
  set_rtld_next_symbol(original_nanosleep, "nanosleep");

  if (getenv("TIMEDILATION") == NULL)
  {
    TIMEDILATION_LOG_VERBOSE("no timedilation environment variable found");
  }
  else
  {
    TIMEDILATION_LOG_VERBOSE("found timedilation environment variable so turning on timedilation");

    std::stringstream input;
    input << getenv("TIMEDILATION");
    input >> timedilation;

    TIMEDILATION_LOG_VERBOSE("parsed timedilation environment variable: '" << getenv("timedilation") << "' -> " << timedilation);
  }

  TIMEDILATION_LOG_VERBOSE("timedilation=" << timedilation);


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

  TIMEDILATION_LOG_VERBOSE("end");
}


void timedilation_finish()
{
  TIMEDILATION_LOG_VERBOSE("begin");

  TIMEDILATION_LOG_VERBOSE("turning off timedilation");
  timedilation = 0;

  TIMEDILATION_LOG_VERBOSE("end");
}

/* --------------------------------------------------------------------------------------------------------------------- */
