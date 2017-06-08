#ifndef EVENTLOOPTESTSUITE_H
#define EVENTLOOPTESTSUITE_H

#include <cppunit/extensions/HelperMacros.h>

/**
 * Test suite for EventLoopServer and EventLoopClient.
 */
class EventLoopTestSuite : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(EventLoopTestSuite);
    CPPUNIT_TEST(timer);
    CPPUNIT_TEST_SUITE_END();

protected:
    void timer();
};

CPPUNIT_TEST_SUITE_REGISTRATION(EventLoopTestSuite);

#endif /* EVENTLOOPTESTSUITE_H */
