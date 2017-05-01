#include "SocketTestSuite.h"

#include <rct/EventLoop.h>
#include <rct/SocketServer.h>

#include <iostream>
#include <thread>

void SocketTestSuite::unixSockets()
{
    const Path unixPathToUse("test_unixSocket");
    unixPathToUse.rm();

    // We need an event loop to receive signals.
    EventLoop::SharedPtr loop(new EventLoop);
    loop->init(EventLoop::MainEventLoop);

    SocketServer s;

    CPPUNIT_ASSERT(!s.isListening());

    std::vector<std::string> eventLog;
    std::mutex m;

    s.newConnection().connect([&](SocketServer *)
                              {
                                  std::lock_guard<std::mutex> g(m);
                                  eventLog.push_back("accepted");
                              });
    s.error().connect([&](SocketServer*, SocketServer::Error e)
                      {
                          std::cerr << "Server error: " << e << std::endl;
                      });

    CPPUNIT_ASSERT(s.listen(unixPathToUse));
    CPPUNIT_ASSERT(s.isListening());

    std::thread serverThread([&](){loop->exec(200);});

    realSleep(100);
    SocketClient::SharedPtr client(new SocketClient(SocketClient::Unix));

    CPPUNIT_ASSERT(client->state() == SocketClient::Disconnected);
    CPPUNIT_ASSERT(client->connect(unixPathToUse));
    realSleep(50);
    client->close();

    serverThread.join();

    CPPUNIT_ASSERT(eventLog.size() == 1);
    CPPUNIT_ASSERT(eventLog[0] == "accepted");
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
