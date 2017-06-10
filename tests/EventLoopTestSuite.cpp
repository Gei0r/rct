#include "EventLoopTestSuite.h"

#include "DeferredAssert.h"

#include <rct/EventLoop.h>
#include <rct/Timer.h>

#include <thread>

static void realSleep(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    timespec s;
    s.tv_sec = ms/1000;
    s.tv_nsec = (ms%1000) * 1000 * 1000;

    while(nanosleep(&s, &s) == -1 && errno == EINTR);
#endif
}

void EventLoopTestSuite::timer()
{
    EventLoop::SharedPtr loop(new EventLoop);
    loop->init(EventLoop::MainEventLoop);

    DeferredAsserter da;

    int timerCalled = 0;

    const int timerId =
        loop->registerTimer([&](int f_timerId) {
                DEFERRED_COMPARE(da, f_timerId, timerId);
                timerCalled++;
            }, 100, Timer::SingleShot);

    loop->exec(200);

    CPPUNIT_ASSERT(timerCalled == 1);
    CPPUNIT_ASSERT(da.result());
}

void EventLoopTestSuite::timer_otherThread()
{
    EventLoop::SharedPtr loop(new EventLoop);
    loop->init(EventLoop::MainEventLoop);

    DeferredAsserter da;

    int timerCalled = 0;
    int timerId = 0;

    std::thread t([&](){
            realSleep(50);
            timerId = loop->registerTimer([&](int f_timerId) {
                    DEFERRED_COMPARE(da, f_timerId, timerId);
                    timerCalled++;
                }, 50, Timer::SingleShot);
        });


    loop->exec(150);

    t.join();

    CPPUNIT_ASSERT(timerCalled == 1);
    CPPUNIT_ASSERT(da.result());
}
