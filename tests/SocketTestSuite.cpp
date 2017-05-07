#include "SocketTestSuite.h"

#include "DeferredAssert.h"

#include <rct/EventLoop.h>
#include <rct/SocketServer.h>

#include <iostream>
#include <thread>
#include <sstream>

void SocketTestSuite::unixSockets()
{
    const Path unixPathToUse("test_unixSocket");
    unixPathToUse.rm();

    DeferredAsserter da;

    // We need an event loop to receive signals.
    EventLoop::SharedPtr loop(new EventLoop);
    loop->init(EventLoop::MainEventLoop);

    SocketServer s;

    CPPUNIT_ASSERT(!s.isListening());

    // flag whether the newConnection slot was called.
    bool gotNewConnection = false;

    // flag whether the connected slot was called.
    bool gotConnected = false;

    // the slot lambdas will be called from the serverThread's context.
    s.newConnection().connect([&](SocketServer *s)
                              {
                                  gotNewConnection = true;

                                  auto client = s->nextConnection();

                                  DEFERRED_COMPARE(da, client->mode(), SocketClient::Unix);
                                  DEFERRED_COMPARE(da, client->state(), SocketClient::Connected);
                              });
    s.error().connect([&](SocketServer*, SocketServer::Error e)
                      {
                          std::ostringstream oss;
                          oss << "Server error: " << e;
                          da.fail(oss.str());
                      });

    CPPUNIT_ASSERT(s.listen(unixPathToUse));
    CPPUNIT_ASSERT(s.isListening());

    std::thread serverThread([&](){loop->exec(200);});

    realSleep(100);
    SocketClient::SharedPtr client(new SocketClient(SocketClient::Unix));

    client->connected().connect([&](SocketClient::SharedPtr)
                                {
                                    gotConnected = true;
                                });

    CPPUNIT_ASSERT(client->state() == SocketClient::Disconnected);
    CPPUNIT_ASSERT(client->connect(unixPathToUse));
    realSleep(50);
    client->close();

    serverThread.join();

    CPPUNIT_ASSERT(gotNewConnection);
    CPPUNIT_ASSERT(gotConnected);
    CPPUNIT_ASSERT(da.result());

}

void SocketTestSuite::realSleep(int ms)
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
