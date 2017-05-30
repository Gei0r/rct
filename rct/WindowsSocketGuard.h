#ifndef WINDOWSSOCKETGUARD_H
#define WINDOWSSOCKETGUARD_H

#include <Winsock2.h>

struct WindowsSocketGuard
{
    WSADATA mData;
    typedef WSADATA value_type;
    WindowsSocketGuard()
    {
        ::WSAStartup(MAKEWORD(2, 2), &mData);
    }
    ~WindowsSocketGuard()
    {
        ::WSACleanup();
    }
    value_type & get() {
        return mData;
    }
};

#endif // WINDOWSSOCKETGUARD_H
