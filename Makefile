CXXFLAGS  = -std=c++17 -Wall -Werror -O2 -ggdb3
CXXFLAGS += -fPIC -DPIC
CXXFLGAS += -D_GNU_SOURCE -fno-omit-frame-pointer -DGNU_SOURCE
CXXFLAGS += $(EXTRA_CXXFLAGS)

LDFLAGS   = -ldl -Wl,--export-dynamic
LDFLAGS  += $(EXTRA_LDFLAGS)

TEST_CXXFLAGS = $(CXXFLAGS)
TEST_LDFLAGS  = $(LDFLAGS)

LIBTIMEDILATION_CXXFLAGS = $(CXXFLAGS) -Wno-nonnull-compare
LIBTIMEDILATION_LDFLAGS  = $(LDFLAGS)


.PHONY: all
all: test libtimedilation.so


.PHONY: run
run: run-test-no-timedilation run-test-with-timedilation

.PHONY: run-test-with-timedilation
run-test-with-timedilation: test libtimedilation.so
	date
	TIMEDILATION=4 LD_PRELOAD=$(PWD)/libtimedilation.so ./test
	date
	echo

.PHONY: run-test-no-timedilation
run-test-no-timedilation: test libtimedilation.so
	date
	LD_PRELOAD=$(PWD)/libtimedilation.so ./test
	date
	echo


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
