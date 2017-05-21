#include "SocketTestSuite.h"

#include "DeferredAssert.h"

#include <rct/EventLoop.h>
#include <rct/SocketServer.h>
#include <rct/Log.h>
#include <rct/Timer.h>

#include <iostream>
#include <thread>
#include <sstream>

void SocketTestSuite::unixSockets()
{
    const Path unixPathToUse("test_unixSocket");

    // need to delete the file if it already exists
    unixPathToUse.rm();

    DeferredAsserter da;

    // We need an event loop to receive signals.
    EventLoop::SharedPtr loop(new EventLoop);
    loop->init(EventLoop::MainEventLoop);

    // create the server
    SocketServer::SharedPtr serverListenSock(new SocketServer);

    // the following variables count how often certain slots are called:
    int server_newConnection = 0;
    int server_recv = 0;
    int client_connected = 0;
    int client_recv = 0;

    // This will hold the client connection that the server gets once a client
    // connects.
    SocketClient::SharedPtr server_socketToClient;

    serverListenSock->newConnection().connect([&](SocketServer *serverListenSock)
        {
            server_newConnection++;

            // important: server_socketToClient needs to survive beyond this lambda,
            // otherwise the connection will be closed on scope exit.
            server_socketToClient = serverListenSock->nextConnection();

            DEFERRED_COMPARE(da, server_socketToClient->mode(), SocketClient::Unix);
            DEFERRED_COMPARE(da, server_socketToClient->state(), SocketClient::Connected);

            server_socketToClient->write("msg from server");

            server_socketToClient->readyRead().connect(
                [&](SocketClient::SharedPtr, Buffer &&b)
                {
                    server_recv++;
                    std::string recv(b.data(), b.end());
                    DEFERRED_COMPARE(da, b.size(), 15u);
                    DEFERRED_COMPARE(da, recv, "msg from client");
                });

            server_socketToClient->error().connect([&](SocketClient::SharedPtr,
                                              SocketClient::Error e)
                {
                    debug() << "Error on server_socketToClient " << e;
                });
        });

    serverListenSock->error().connect([&](SocketServer*, SocketServer::Error e)
                       {
                           std::ostringstream oss;
                           oss << "Server error: " << e;
                           da.fail(oss.str());
                       });

    CPPUNIT_ASSERT(serverListenSock->listen(unixPathToUse));

    // The server is now set up and once we start th event loop, it will listen
    // for new connections.
    // We now set up two timers.
    // In the first timer, we create a client that tries to connect to the
    // SocketServer. Once the connection is sucessful, we send "msg from
    // client". We also expect to get the message "msg from server" once we
    // connect to the server.
    // In the first timer, we expect that the connection is still alive on both
    // ends. We then close the connection on one end.
    // We expect that the connection is closed on BOTH ends when the EventLoop
    // finishes.

    SocketClient::SharedPtr client;
    loop->registerTimer([&](int /*timerid*/)
        {
            client = SocketClient::SharedPtr(new SocketClient(SocketClient::Unix));

            client->connected().connect([&](SocketClient::SharedPtr c)
                {
                    client_connected++;
                    c->write("msg from client");
                });

            client->readyRead().connect([&](SocketClient::SharedPtr, Buffer &&b)
                {
                    // we expect to get some data upon connection to the server
                    client_recv++;
                    std::string receivedData(b.data(), b.end());
                    DEFERRED_COMPARE(da, receivedData, "msg from server");
                });

            client->error().connect([&](SocketClient::SharedPtr, SocketClient::Error e)
                {   // shouldn't happen
                    debug() << "client " << " got error " << e;
                });

            client->disconnected().connect([&](SocketClient::SharedPtr)
                {   // shouldn't happen
                    debug() << "Client disconnected";
                });

            // now that the callbacks are all set up, we can actually connect.
            DEFERRED_ASSERT(da, client->connect(unixPathToUse));
        },
        10, Timer::SingleShot);

    loop->registerTimer([&](int /*timerid*/)
        {
            // should still be connected
            DEFERRED_COMPARE(da, client->state(),                SocketClient::Connected);
            DEFERRED_COMPARE(da, server_socketToClient->state(), SocketClient::Connected);

            client->close();
        }, 100, Timer::SingleShot);

    // now that all the callbacks and all the timers are set up, we set the
    // whole thing in motion.
    loop->exec(200);

    // In the second timer (timeout 100 ms), we disconnected the client end of
    // the connection. Now both ends should be in disconnected state.
    CPPUNIT_ASSERT(client->state() == SocketClient::Disconnected);
    CPPUNIT_ASSERT(server_socketToClient->state() == SocketClient::Disconnected);

    CPPUNIT_ASSERT(server_newConnection == 1);
    CPPUNIT_ASSERT(client_connected == 1);
    CPPUNIT_ASSERT(client_recv == 1);
    CPPUNIT_ASSERT(server_recv == 1);
    CPPUNIT_ASSERT(da.result());
}
