#include "glibc_versions.h"

#include <sstream>

#include <dlfcn.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/* --------------------------------------------------------------------------------------------------------------------- */

static unsigned int timedilation = 0;

/* --------------------------------------------------------------------------------------------------------------------- */

static bool timedilation_verbose = false;

#define TIMEDILATION_LOG_VERBOSE(message)                          \
  do {                                                             \
    if (timedilation_verbose)                                      \
    {                                                              \
      std::stringstream m;                                         \
      m << message;                                                \
      printf("timedilation: %s: %s\n", __func__, m.str().c_str()); \
    }                                                              \
  } while(0)

/* --------------------------------------------------------------------------------------------------------------------- */

#define TIMEDILATION_SYMBOL_DEFINITION(symbol_name, return_type, input_args) \
    extern "C"                                                               \
    {                                                                        \
      typedef return_type (*symbol_name ## _signature)input_args;            \
      static symbol_name ## _signature  original_ ## symbol_name = NULL;     \
    }                                                                        \
                                                                             \
    return_type symbol_name input_args


static timespec  initial_clock_gettime[16];
static timeval   initial_gettimeofday;
static time_t    initial_time;


#include "timedilation_epoll.cpp"
#include "timedilation_poll.cpp"
#include "timedilation_pthread.cpp"
#include "timedilation_select.cpp"
#include "timedilation_sleep.cpp"
#include "timedilation_time.cpp"
#include "timedilation_timer.cpp"
#include "timedilation_timerfd.cpp"

/* --------------------------------------------------------------------------------------------------------------------- */

/**
 * If the environment variable do not exist yet we are parent process so set the variable.
 *
 * If the environment variable exist read it and use it because we are subprocess.
 */
template<typename T>
void get_set_initial_value(const char* name, T& value)
{
  if (getenv(name) != NULL)
  {
    std::stringstream input;
    input << getenv(name);
    input >> value;

    TIMEDILATION_LOG_VERBOSE(name << ": loaded " << value << " from '" << getenv(name) << "'");

    return;
  }

  std::stringstream output;
  output << value;
  setenv(name, output.str().c_str(), /* overwrite = */ 0);

  TIMEDILATION_LOG_VERBOSE(name << ": saved " << value << " as '" << getenv(name) << "'");
}

/* --------------------------------------------------------------------------------------------------------------------- */

/**
 * Get newest symbol available including newest versioned glibc symbols.
 */
template<typename T>
void set_rtld_next_symbol(T& t, const char* name)
{
  TIMEDILATION_LOG_VERBOSE("dlsym(RTLD_NEXT, " << name << ")");

  t = (T) dlsym(RTLD_NEXT, name);
  if (t != NULL)
  {
    TIMEDILATION_LOG_VERBOSE("success " << name);
    return;
  }

  for (const char* glibc_version : glibc_versions)
  {
    TIMEDILATION_LOG_VERBOSE("dlvsym(RTLD_NEXT, " << name << ", " << glibc_version << ")");

    t = (T) dlvsym(RTLD_NEXT, name, glibc_version);
    if (t != NULL)
    {
      TIMEDILATION_LOG_VERBOSE("success " << name << " (" << glibc_version << ")");
      return;
    }
  }

  TIMEDILATION_LOG_VERBOSE("original symbol for symbol '" << name << "' not found");
}

/* --------------------------------------------------------------------------------------------------------------------- */

extern "C" {

void timedilation_init()  __attribute__((constructor));
void timedilation_finish() __attribute__((destructor));

} // extern "C"

/* --------------------------------------------------------------------------------------------------------------------- */

#define TIMEDILATION_INITIALIZE_SYMBOL(symbol_name) set_rtld_next_symbol(original_ ## symbol_name, #symbol_name)

void timedilation_init()
{
  if (getenv("TIMEDILATION_VERBOSE"))
  {
    timedilation_verbose = true;
    TIMEDILATION_LOG_VERBOSE("TIMEDILATION_VERBOSE=" << getenv("TIMEDILATION_VERBOSE"));
  }

  TIMEDILATION_LOG_VERBOSE("begin");

  TIMEDILATION_INITIALIZE_SYMBOL(time);
  TIMEDILATION_INITIALIZE_SYMBOL(gettimeofday);
  TIMEDILATION_INITIALIZE_SYMBOL(clock_gettime);

  TIMEDILATION_INITIALIZE_SYMBOL(timer_create);
  TIMEDILATION_INITIALIZE_SYMBOL(timer_settime);
  TIMEDILATION_INITIALIZE_SYMBOL(timer_gettime);
  TIMEDILATION_INITIALIZE_SYMBOL(timer_delete);

  TIMEDILATION_INITIALIZE_SYMBOL(timerfd_create);
  TIMEDILATION_INITIALIZE_SYMBOL(timerfd_settime);
  TIMEDILATION_INITIALIZE_SYMBOL(timerfd_gettime);

  TIMEDILATION_INITIALIZE_SYMBOL(close);

  TIMEDILATION_INITIALIZE_SYMBOL(pthread_cond_clockwait);
  TIMEDILATION_INITIALIZE_SYMBOL(pthread_cond_timedwait);

  TIMEDILATION_INITIALIZE_SYMBOL(select);
  TIMEDILATION_INITIALIZE_SYMBOL(pselect);

  TIMEDILATION_INITIALIZE_SYMBOL(poll);
  TIMEDILATION_INITIALIZE_SYMBOL(ppoll);

  TIMEDILATION_INITIALIZE_SYMBOL(epoll_wait);
  TIMEDILATION_INITIALIZE_SYMBOL(epoll_pwait);

  TIMEDILATION_INITIALIZE_SYMBOL(sleep);
  TIMEDILATION_INITIALIZE_SYMBOL(usleep);
  TIMEDILATION_INITIALIZE_SYMBOL(nanosleep);

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

/* --------------------------------------------------------------------------------------------------------------------- */

void timedilation_finish()
{
  TIMEDILATION_LOG_VERBOSE("begin");

  TIMEDILATION_LOG_VERBOSE("turning off timedilation");
  timedilation = 0;

  TIMEDILATION_LOG_VERBOSE("end");
}

/* --------------------------------------------------------------------------------------------------------------------- */
