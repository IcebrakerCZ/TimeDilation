#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

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


std::atomic<bool> timer_handler_called = false;

void timer_handler(int sig, siginfo_t *si, void *uc)
{
  timer_handler_called = true;
}


#include <execinfo.h>

int main(int  argc, char *  argv[])
{
  {
    ScopeDurationPrinter sd("sleep");

    sleep(2);
  }


  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = &timer_handler;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGUSR1, &sa, NULL) != 0)
  {
    std::cout << "ERROR: sigaction: " << strerror(errno) << std::endl;
    return EXIT_FAILURE;
  }

  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  if (sigprocmask(SIG_UNBLOCK, &mask, NULL) != 0)
  {
    std::cout << "ERROR: sigprocmask: " << strerror(errno) << std::endl;
    return EXIT_FAILURE;
  }

  timer_t timer;
  sigevent sev;
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIGUSR1;
  sev.sigev_value.sival_ptr = &timer;

  if (timer_create(CLOCK_REALTIME, &sev, &timer) != 0)
  {
    std::cout << "ERROR: timer_create: " << strerror(errno) << std::endl;
    return EXIT_FAILURE;
  }

  {
    ScopeDurationPrinter sd("timer");

    itimerspec timeout = {/*it_interval = */ {0, 0}, /* it_value = */ {2, 0}};
    if (timer_settime(timer, 0, &timeout, NULL) != 0)
    {
      std::cout << "ERROR: timer_settime: " << strerror(errno) << std::endl;
      return EXIT_FAILURE;
    }

    while (!timer_handler_called)
    {
    }
  }

  if (timer_delete(timer) != 0)
  {
    std::cout << "ERROR: timer_delete: " << strerror(errno) << std::endl;
    return EXIT_FAILURE;
  }


  int fd = timerfd_create(CLOCK_REALTIME, 0);
  if (fd < 0)
  {
    std::cout << "ERROR: timerfd_create: " << strerror(errno) << std::endl;
    return EXIT_FAILURE;
  }

  {
    ScopeDurationPrinter sd("timerfd");

    itimerspec timeout = {.it_interval = {.tv_sec = 0, .tv_nsec = 0}, .it_value = {.tv_sec = 2, .tv_nsec = 0}};
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
