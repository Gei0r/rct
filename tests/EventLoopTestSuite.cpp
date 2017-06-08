#include "EventLoopTestSuite.h"

#include "DeferredAssert.h"

#include <rct/EventLoop.h>
#include <rct/Timer.h>

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
            }, 10, Timer::SingleShot);

    loop->exec(50);

    CPPUNIT_ASSERT(timerCalled == 1);
    CPPUNIT_ASSERT(da.result());
}
