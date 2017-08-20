#ifndef WINDOWSSOCKETGLOBALGUARD_H
#define WINDOWSSOCKETGLOBALGUARD_H

#include <rct/WindowsSocketGuard.h>

struct WindowsSocketGlobalGuard
{
    typedef WindowsSocketGuard::value_type value_type;
    static WindowsSocketGuard mGuard;
    static value_type & get();
};

#endif // WINDOWSSOCKETGLOBALGUARD_H
