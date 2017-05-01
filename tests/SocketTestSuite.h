#ifndef SOCKETTESTSUITE_H
#define SOCKETTESTSUITE_H

#include <cppunit/extensions/HelperMacros.h>

/**
 * Test suite for SocketServer and SocketClient.
 */
class SocketTestSuite : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(SocketTestSuite);
    CPPUNIT_TEST(unixSockets);
    CPPUNIT_TEST_SUITE_END();

protected:
    /**
     * Test creation and communication through unix domain sockets
     */
    void unixSockets();

private:
    void realSleep(int time_ms);
};

CPPUNIT_TEST_SUITE_REGISTRATION(SocketTestSuite);

#endif /* SOCKETTESTSUITE_H */
