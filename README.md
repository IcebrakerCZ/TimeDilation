Time Dilation Library

GIT:
        https://github.com/IcebrakerCZ/TimeDilation.git

NOTES:
        Library dilating time in specified binary and its child processes
        so the time dilated processes thinks thay runs multiple times faster.
        
        It's useful in slow environments like running a binary under valgrind
        when the binary logic is time sensitive e.g. using alarms, sleep
        async reading/writing with timeouts etc.
        
        Time dilated symbols:
        
            epoll_wait
            epoll_pwait
            poll
            ppoll
            pthread_cond_clockwait
            pthread_cond_timedwait
            select
            pselect
            sleep
            usleep
            nanosleep
            clock_gettime
            gettimeofday
            time
            timer_create
            timer_settime
            timer_gettime
            timer_delete
            timerfd_create
            timerfd_settime
            timerfd_gettime
            close
        
        The library saves actual time when first loaded in the parent process
        and then uses the actual time and saved initial time to make the process
        believe that the time flow faster then it really flows in the slow environment.
        
        For example unittest checking requests processing like below:
        
            TEST_F(MyBinaryTest, CheckMyLogicPerformance)
            {
              time_t start = time(NULL);
              process_requests();
              time_t stop = time(NULL);

              // Check that the logic takes 2 seconds at most.
              ASSERT_LE(stop - start, 2);
            }
        
        will PASS under normally conditions but can FAIL under valgrind becaus the 'process_requests()'
        call can take more than 2 seconds maybe even 10 seconds or more based on amount of allocations.
        
        In this scenario it is best to run the test like:
        
            timedilation -n 8 valgrind --tool=memcheck .... ./unittest
        
        The timedilation utility will slow down the time 4x so maybe the 'process_requsts()' duration
        will be 4x smaller than it really took so the test will PASS as usual because the time is
        redefined as:
        
            time_t time()
            {
              time_t tv = original_time(t);

              if (timedilation)
              {
                tv = initial_time + ((tv - initial_time) / timedilation);
              }

              return tv;
            }
        
        so the time is slowed down by TIMEDILATION multiple but proportionally to initial_time.

LICENSE:
        LGPLv3+

AUTHORS:
        Jan Horak

REQUIREMENTS:
        Recent g++, Linux
