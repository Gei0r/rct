#include "SocketTestSuite.h"

#include "DeferredAssert.h"

#include <rct/EventLoop.h>
#include <rct/SocketServer.h>
#include <rct/Log.h>

#include <thread>

void SocketTestSuite::unixSockets()
{
    const Path unixPathToUse("test_unixSocket");

    // need to delete the file if it already exists
    unixPathToUse.rm();

    // We need an event loop to receive signals.
    EventLoop::SharedPtr loop(new EventLoop);
    loop->init(EventLoop::MainEventLoop);

    // create the server
    SocketServer::SharedPtr s(new SocketServer);

    // This will hold the client connection that the server gets once a client
    // connects.
    SocketClient::SharedPtr serverConnection;

    s->newConnection().connect([&](SocketServer *s)
        {
            // important: serverConnection needs to survive beyond this lambda,
            // otherwise the connection will be closed on scope exit.
            serverConnection = s->nextConnection();

            // by this point, the client will already have gotten the HUP (hang
            // up), so they don't receive the message :(
            serverConnection->write("msg frm server");
        });

    CPPUNIT_ASSERT(s->listen(unixPathToUse));

    std::thread serverThread([&](){loop->exec(300);});

    realSleep(100);
    SocketClient::SharedPtr client(new SocketClient(SocketClient::Unix));

    int numReadsFromClient = 0;
    client->readyRead().connect([&](SocketClient::SharedPtr, Buffer &&b)
        {
            std::string receivedData(b.data(), b.end());
            debug() << "client received " << receivedData;
            numReadsFromClient++;
        });

    CPPUNIT_ASSERT(client->connect(unixPathToUse));

    realSleep(100);

    serverThread.join();

    CPPUNIT_ASSERT(numReadsFromClient == 1);
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
