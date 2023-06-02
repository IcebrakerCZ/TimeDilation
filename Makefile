CXXFLAGS  = -std=c++17 -Wall -Werror -O2 -ggdb3
CXXFLAGS += -fPIC -DPIC
CXXFLGAS += -D_GNU_SOURCE -fno-omit-frame-pointer -DGNU_SOURCE
CXXFLAGS += $(EXTRA_CXXFLAGS)

LDFLAGS   = -ldl -Wl,--export-dynamic
LDFLAGS  += $(EXTRA_LDFLAGS)


.PHONY: all
all: test libtimedilation.so


.PHONY: run
run: run-test

.PHONY: run-test
run-test: test
	date
	TIMESHIFT=5 LD_PRELOAD=$(PWD)/libtimedilation.so ./test
	date


.PHONY: clean
clean:
	rm -f test $(TEST_OBJS)
	rm -f libtimedilation.so $(TIMEDILATION_OBJS)


TEST_SRCS = test.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
$(TEST_OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test: $(TEST_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)


LIBTIMEDILATION_SRCS = timedilation.cpp
LIBTIMEDILATION_OBJS = $(TIMEDILATION_SRCS:.cpp=.o)
$(LIBTIMEDILATION_OBJS): %.o: %.cpp
	$(CXX) $(LIBTIMEDILATION_CXXFLAGS) -c -o $@ $<


libtimedilation.so: $(LIBTIMEDILATION_OBJS) timespec.h timeval.h
	$(CXX) -o $@ $^ -shared $(LIBTIMEDILATION_LDFLAGS)
