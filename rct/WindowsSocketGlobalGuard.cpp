#include <rct/WindowsSocketGlobalGuard.h>
WindowsSocketGuard WindowsSocketGlobalGuard::mGuard;

/* static */ WindowsSocketGlobalGuard::value_type &WindowsSocketGlobalGuard::get()
{
    return mGuard.get();
}
