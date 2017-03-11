#ifndef STRINGTESTSUITE_H
#define STRINGTESTSUITE_H

#include <cppunit/extensions/HelperMacros.h>

class StringTestSuite : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(StringTestSuite);
    CPPUNIT_TEST(longLongFormatTest);
    CPPUNIT_TEST_SUITE_END();

protected:
    void longLongFormatTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StringTestSuite);


#endif /* STRINGTESTSUITE_H */
