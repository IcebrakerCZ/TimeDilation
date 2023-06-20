#include "glibc_versions.h"

#include <dlfcn.h>

/* --------------------------------------------------------------------------------------------------------------------- */

#define TIMEDILATION_SYMBOL_DEFINITION(symbol_name, return_type, input_args) \
    extern "C"                                                               \
    {                                                                        \
      typedef return_type (*symbol_name ## _signature)input_args;            \
      static symbol_name ## _signature  original_ ## symbol_name = NULL;     \
    }                                                                        \
                                                                             \
    return_type symbol_name input_args

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

#define TIMEDILATION_INITIALIZE_SYMBOL(symbol_name) set_rtld_next_symbol(original_ ## symbol_name, #symbol_name)

/* --------------------------------------------------------------------------------------------------------------------- */
