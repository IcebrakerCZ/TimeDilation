#include <chrono>
#include <cstdlib>
#include <iostream>

#include <unistd.h>
#include <sys/timerfd.h>
#include <errno.h>
#include <string.h>

struct ScopeDurationPrinter
{
  ScopeDurationPrinter(const std::string& name)
    : m_name(name)
  {}

  ~ScopeDurationPrinter()
  {
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

    std::chrono::milliseconds duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start);

    std::cout << m_name << " duration: " << duration_ms.count() << " ms" << std::endl;
  }

  std::string m_name;

  std::chrono::system_clock::time_point m_start = std::chrono::system_clock::now();
};

#include <execinfo.h>

int main(int  argc, char *  argv[])
{
  {
    ScopeDurationPrinter sd("sleep");

    sleep(2);
  }


  int fd = timerfd_create(CLOCK_REALTIME, 0);
  if (fd < 0)
  {
    std::cout << "ERROR: timerfd_create: " << strerror(errno) << std::endl;
    return EXIT_FAILURE;
  }

  {
    ScopeDurationPrinter sd("timerfd");

    itimerspec timeout = {/*it_interval = */ {0, 0}, /* it_value = */ {2, 0}};
    if (timerfd_settime(fd, 0, &timeout, NULL) < 0)
    {
      std::cout << "ERROR: timerfd_settime: " << strerror(errno) << std::endl;
      return EXIT_FAILURE;
    }

    uint8_t buf[sizeof(uint64_t)];
    if (read(fd, &buf, sizeof(buf)) != sizeof(uint64_t))
    {
      std::cout << "ERROR: read: " << strerror(errno) << std::endl;
      return EXIT_FAILURE;
    }
  }

  if (close(fd) < 0)
  {
    std::cout << "ERROR: close: " << strerror(errno) << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
