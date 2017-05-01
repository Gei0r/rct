#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <memory>
#include <queue>
#include <stdint.h>

#include <rct/Path.h>
#include <rct/SignalSlot.h>
#include <rct/SocketClient.h>

struct sockaddr;

/**
 * Represents a TCP or Unix Domain Socket Server.
 *
 * You can start the server by calling one of the listen() methods. When a
 * client connects, the newConnection signal is emitted and the server
 * continues listening for new connections.
 *
 * Upon receiving a newConnectionSignal, you can retreive the newly connected
 * Client through the nextConnection() method.
 *
 * This class needs a global running EventLoop.
 *
 * On Windows, the Unix Domain Socket server is emulated through TCP.
 */
class SocketServer
{
public:
    typedef std::shared_ptr<SocketServer> SharedPtr;
    typedef std::weak_ptr<SocketServer> WeakPtr;

    SocketServer();

    /**
     * Destructor.
     *
     * Closes the socket server.
     */
    ~SocketServer();

    enum Mode { IPv4, IPv6 };

    void close();
    bool listen(uint16_t port, Mode mode = IPv4); // TCP
#ifndef _WIN32
    bool listen(const Path &path); // UNIX
    bool listenFD(int fd);         // UNIX
#endif
    bool isListening() const { return fd != -1; }

    /**
     * Returns a newly accepted client connection and removes it from the
     * internal list.
     *
     * @return nullptr if there are no more newly accepted client connections.
     */
    SocketClient::SharedPtr nextConnection();

    Signal<std::function<void(SocketServer*)> >& newConnection() { return serverNewConnection; }

    enum Error { InitializeError, BindError, ListenError, AcceptError };
    Signal<std::function<void(SocketServer*, Error)> >& error() { return serverError; }

private:
    void socketCallback(int fd, int mode);
    bool commonBindAndListen(sockaddr* addr, size_t size);
    bool commonListen();

private:
    int fd;
    bool isIPv6;
    Path path;
    std::queue<int> accepted;
    Signal<std::function<void(SocketServer*)> > serverNewConnection;
    Signal<std::function<void(SocketServer*, Error)> > serverError;
};

#endif
