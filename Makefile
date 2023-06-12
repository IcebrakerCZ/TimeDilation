CXXFLAGS  = -m64 -std=c++17 -Wall -Werror -O2 -ggdb3
CXXFLGAS += -fno-omit-frame-pointer
CXXFLAGS += $(EXTRA_CXXFLAGS)

LDFLAGS   = -Wl,--export-dynamic
LDFLAGS  += $(EXTRA_LDFLAGS)

TEST_CXXFLAGS = $(CXXFLAGS)
TEST_LDFLAGS  = $(LDFLAGS) -lrt

LIBTIMEDILATION_CXXFLAGS = $(CXXFLAGS) -pthread -fPIC -DGNU_SOURCE
LIBTIMEDILATION_LDFLAGS  = $(LDFLAGS)  -pthread -lrt -ldl


TEST_TIMEDILATION ?= 4


.PHONY: all
all: test libtimedilation.so


.PHONY: run
run: run-test-without-timedilation run-test-with-timedilation

.PHONY: run-test-with-timedilation
run-test-with-timedilation: test libtimedilation.so
	@START=`date +%s`; \
	echo "timedilation: $(TEST_TIMEDILATION)x"; \
	TIMEDILATION=$(TEST_TIMEDILATION) LD_PRELOAD=$(PWD)/libtimedilation.so ./test; \
	END=`date +%s`; \
	DURATION=`expr $$END - $$START`; \
	echo "duration: $$DURATION s"
	@echo

.PHONY: run-test-without-timedilation
run-test-without-timedilation: test libtimedilation.so
	@START=`date +%s`; \
	echo "timedilation: disabled"; \
	LD_PRELOAD=$(PWD)/libtimedilation.so ./test; \
	END=`date +%s`; \
	DURATION=`expr $$END - $$START`; \
	echo "duration: $$DURATION s"
	@echo


.PHONY: clean
clean:
	rm -f test $(TEST_OBJS)
	rm -f libtimedilation.so $(LIBTIMEDILATION_OBJS)
	rm -f glibc_versions.h


TEST_SRCS = test.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
$(TEST_OBJS): %.o: %.cpp
	$(CXX) $(TEST_CXXFLAGS) -c -o $@ $<

test: $(TEST_OBJS)
	$(CXX) -o $@ $^ $(TEST_LDFLAGS)


LIBTIMEDILATION_SRCS = timedilation.cpp
LIBTIMEDILATION_OBJS = $(LIBTIMEDILATION_SRCS:.cpp=.o)
$(LIBTIMEDILATION_OBJS): %.o: %.cpp timespec.h timeval.h glibc_versions.h $(wildcard timedilation-*.cpp)
	$(CXX) $(LIBTIMEDILATION_CXXFLAGS) -c -o $@ $<


libtimedilation.so: $(LIBTIMEDILATION_OBJS)
	$(CXX) -o $@ $^ -shared $(LIBTIMEDILATION_LDFLAGS)


glibc_versions.h:
	strings /lib/x86_64-linux-gnu/libc.so.6                                      \
	    | grep GLIBC_                                                            \
	    | grep -v GLIBC_PRIVATE                                                  \
	    | sort -rV                                                               \
	    | sed -E -e '1 { s/^(.*)$$/const char* glibc_versions[] = { "\1"/ }'     \
	             -e '2,999 { s/^(.*)$$/                               , "\1"/ }' \
	    > $@
	echo "                              };" >> $@
