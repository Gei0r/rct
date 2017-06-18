#include <cppunit/extensions/HelperMacros.h>

class PathTestSuite : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(PathTestSuite);

#ifdef _WIN32
    CPPUNIT_TEST(testPathConstructionWindows);
    CPPUNIT_TEST(testPathStatusWindows);
    CPPUNIT_TEST(testRelativeToAbsPath_windows);
#endif

    CPPUNIT_TEST(mkdirAndRmdir);
    CPPUNIT_TEST(unicode);
    CPPUNIT_TEST(renameFile);
    CPPUNIT_TEST(renameFileToExisting);
    CPPUNIT_TEST(renameDir);
    CPPUNIT_TEST(renameDir_unicode);
    CPPUNIT_TEST_SUITE_END();

protected:
#ifdef _WIN32
    void testPathConstructionWindows();
    void testPathStatusWindows();
    void testRelativeToAbsPath_windows();
#endif
    void mkdirAndRmdir();

    void unicode();

    void renameFile();
    void renameFileToExisting();
    void renameDir();
    void renameDir_unicode();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PathTestSuite);
