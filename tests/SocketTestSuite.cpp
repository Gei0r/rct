#include "SocketTestSuite.h"

#include "DeferredAssert.h"

#include <rct/EventLoop.h>
#include <rct/SocketServer.h>
#include <rct/Log.h>

#include <iostream>
#include <thread>
#include <sstream>

void SocketTestSuite::unixSockets()
{
    const Path unixPathToUse("test_unixSocket");
    unixPathToUse.rm();

    DeferredAsserter da;
    std::mutex m;

    // We need an event loop to receive signals.
    EventLoop::SharedPtr loop(new EventLoop);
    loop->init(EventLoop::MainEventLoop);

    SocketServer::SharedPtr s(new SocketServer);

    CPPUNIT_ASSERT(!s->isListening());


    // the following variables count how often certain slots are called:
    int server_newConnection = 0;
    int server_recv = 0;
    int client_connected = 0;
    int client_recv = 0;
    SocketClient::SharedPtr serverSocket;

    // the slot lambdas will be called from the serverThread's context.
    s->newConnection().connect([&](SocketServer *s)
        {
            server_newConnection++;

            // important: serverSocket needs to survive beyond this lambda,
            // otherwise the connection will be closed in its dtor.
            serverSocket = s->nextConnection();

            DEFERRED_COMPARE(da, serverSocket->mode(), SocketClient::Unix);
            DEFERRED_COMPARE(da, serverSocket->state(), SocketClient::Connected);

            serverSocket->write("msg frm server");

            serverSocket->readyRead().connect(
                [&](SocketClient::SharedPtr ptr, Buffer &&b)
                {
                    server_recv++;
                    debug() << "Server received from socket " << ptr.get()
                            << " fd=" << ptr->fd;
                    std::string recv(b.data(), b.end());
                    DEFERRED_COMPARE(da, b.size(), 15u);
                    DEFERRED_COMPARE(da, recv, "msg from client");
                });
        });

    s->error().connect([&](SocketServer*, SocketServer::Error e)
                       {
                           std::ostringstream oss;
                           oss << "Server error: " << e;
                           da.fail(oss.str());
                       });

    CPPUNIT_ASSERT(s->listen(unixPathToUse));
    CPPUNIT_ASSERT(s->isListening());

    std::thread serverThread([&](){loop->exec(300);});

    realSleep(100);
    debug() << "creating client...";
    SocketClient::SharedPtr client(new SocketClient(SocketClient::Unix));
    debug() << "client created." << client.get();

    client->connected().connect([&](SocketClient::SharedPtr c)
        {
            std::lock_guard<std::mutex> g(m);
            debug() << "Client " << c.get()
                    << " connected. Send message...";
            client_connected++;
            c->write("msg from client");
        });
    client->readyRead().connect([&](SocketClient::SharedPtr, Buffer &&b)
        {
            debug() << "client recv";
            std::lock_guard<std::mutex> g(m);
            client_recv++;
            std::string receivedData(b.data(), b.end());
            DEFERRED_COMPARE(da, receivedData, "msg from server");
        });
    client->error().connect([&](SocketClient::SharedPtr ptr,
                                SocketClient::Error e)
        {
            debug() << "client " << ptr.get() << " got error " << e;
        });

    client->disconnected().connect([&](SocketClient::SharedPtr ptr)
        {
            debug() << "Client " << ptr.get() << " disconnected";
        });

    CPPUNIT_ASSERT(client->state() == SocketClient::Disconnected);

    debug() << "client about to connect to " << unixPathToUse << "...";
    CPPUNIT_ASSERT(client->connect(unixPathToUse));
    debug() << "client connect done";

    realSleep(100);
    // client->close();

    serverThread.join();

    CPPUNIT_ASSERT(server_newConnection == 1);
    CPPUNIT_ASSERT(client_connected == 1);
    CPPUNIT_ASSERT(client_recv == 1);
    CPPUNIT_ASSERT(server_recv == 1);
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
