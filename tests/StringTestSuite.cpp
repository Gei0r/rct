#include "StringTestSuite.h"

#include <rct/String.h>

#include <cinttypes>

void StringTestSuite::longLongFormatTest()
{
    // this test case is especially relevant for mingw on windows.
    // Mingw does not supply its own runtime environment, but uses the one
    // supplied by windows (msvcrt).
    // Msvcrt used to not support the "long long" format specifiers, so mingw
    // issues a warning when such a specifier is used.
    // Newer versions of the msvcrt *do* support the specifiers however, so the
    // warning is useless.
    // This test case checks if "long long" format specifiers actually work.

    typedef long long unsigned int llu;
    typedef long long   signed int lli;

    lli i_one = 1;
    llu u_one = 1;

    CPPUNIT_ASSERT(String::format("%lli", i_one) == "1");
    CPPUNIT_ASSERT(String::format("%llu", u_one) == "1");

    uint64_t u64_1   = 1;
    uint64_t u64_ff  = 0xffffffffffffffff;
    uint64_t u64_num = 0xbadeaffecafeaffe;

    CPPUNIT_ASSERT(String::format("#%" PRIx64 "#", u64_1)   == "#1#");
    CPPUNIT_ASSERT(String::format("#%" PRIx64 "#", u64_ff)  == "#ffffffffffffffff#");
    CPPUNIT_ASSERT(String::format("#%" PRIx64 "#", u64_num) == "#badeaffecafeaffe#");

    size_t st = 12;
    CPPUNIT_ASSERT(String::format("#%zu#", st) == "#12#");
}
