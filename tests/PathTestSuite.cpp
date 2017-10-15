#include "PathTestSuite.h"

#include <rct/Path.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <functional>

#ifdef _WIN32
#  include <Windows.h>
#endif

#ifdef _WIN32

class CwdChanger
{
public:
    CwdChanger(const char *newCwd)
    {
        getcwd(&oldCwd[0], _MAX_PATH);
        chdir(newCwd);
    }

    ~CwdChanger() { chdir(oldCwd); }
private:
    char oldCwd[_MAX_PATH];
};

template <class F>
class OnScopeExit
{
public:
    OnScopeExit(const F &ff) : f(ff) {}
    ~OnScopeExit() {f();}
private:
    F f;
};

void PathTestSuite::testPathConstructionWindows()
{
    {
        Path p;
        CPPUNIT_ASSERT_EQUAL(p.size(), (size_t)0);
        CPPUNIT_ASSERT(!p.isAbsolute());
    }

    {
        Path p("file.txt");
        CPPUNIT_ASSERT(p == "file.txt");
        CPPUNIT_ASSERT(!p.isAbsolute());
    }

    {
        Path p("C:\\temp\\test.doc");
        CPPUNIT_ASSERT(p == "C:/temp/test.doc");
        CPPUNIT_ASSERT(p.isAbsolute());
    }

    {
        Path p("\\\\127.0.0.1\\share");
        CPPUNIT_ASSERT(p == "\\\\127.0.0.1/share");
        CPPUNIT_ASSERT(p.isAbsolute());
    }

    {
        Path p("/C/windows");
        CPPUNIT_ASSERT(p == "C:/windows");
        CPPUNIT_ASSERT(p.isAbsolute());
        CPPUNIT_ASSERT(p.isDir());
    }

    {
        // path with colon in it
        Path p("C:\\ab:cd");
        CPPUNIT_ASSERT(p == "C:/ab_colon_cd");
    }

    {
        // relative path with colon in it
        Path p("a:colon_in_the_filename");
        CPPUNIT_ASSERT(p == "a_colon_colon_in_the_filename");
    }

    {
        // drive letter should always be upper case.
        Path p1("c:\\Windows");
        CPPUNIT_ASSERT(p1 == "C:/Windows");
        Path p2("C:\\Windows");
        CPPUNIT_ASSERT(p2 == "C:/Windows");
    }
}

void PathTestSuite::testPathStatusWindows()
{
    // This test checks file exists, check if path is dir or regular file, etc.

    {
        Path p("C:\\windows");
        CPPUNIT_ASSERT(p.exists());
        CPPUNIT_ASSERT(p.isDir());
    }

    {
        Path p("this_file_does_not_exist.not_there");
        CPPUNIT_ASSERT(!p.exists());
    }
}

void PathTestSuite::testRelativeToAbsPath_windows()
{
    CwdChanger toSys32("C:\\windows\\system32");

    Path notepad("notepad.exe");

    CPPUNIT_ASSERT(notepad.exists());
    CPPUNIT_ASSERT(notepad.isFile());

    CPPUNIT_ASSERT(!notepad.isAbsolute());
    bool changed;
    CPPUNIT_ASSERT(notepad.resolve(Path::RealPath, Path(), &changed));
    CPPUNIT_ASSERT(changed);
    CPPUNIT_ASSERT(notepad == "C:/windows/system32/notepad.exe");
    CPPUNIT_ASSERT(notepad.parentDir() == "C:/windows/system32/");
}

#endif  // _WIN32

void PathTestSuite::mkdirAndRmdir()
{
    CPPUNIT_ASSERT(Path::mkdir("temp_subdir"));

    // test if dir exists
    FILE *subdir_file = fopen("temp_subdir/file.txt", "w");
    CPPUNIT_ASSERT(subdir_file);
    if(subdir_file) fclose(subdir_file);

    CPPUNIT_ASSERT(Path::rmdir("temp_subdir"));

    // dir should not exist anymore
    subdir_file = fopen("temp_subdir/file.txt", "w");
    CPPUNIT_ASSERT(!subdir_file);


    // try to create recursive path without setting recursive mode
    // (should fail)
    CPPUNIT_ASSERT(!Path::mkdir("temp_subdir2/anotherSubDir"));


    // try to create recursive path *with* setting recursive mode (should work)
    CPPUNIT_ASSERT(Path::mkdir("temp_subdir3/anotherSubDir", Path::Recursive));

    // check if dir was created
    subdir_file = fopen("temp_subdir3/anotherSubDir/file.txt", "w");

    CPPUNIT_ASSERT(subdir_file);

    if(subdir_file) fclose(subdir_file);

    CPPUNIT_ASSERT(Path::rmdir("temp_subdir3"));

    //file should not exist anymore
    subdir_file = fopen("temp_subdir3/anotherSubdir/file.txt", "w");
    CPPUNIT_ASSERT(!subdir_file);


    // try to rm a non-existing directory
    CPPUNIT_ASSERT(!Path::rmdir("thisDirDoesNotExist"));

    // try to create an already-existing dir
    CPPUNIT_ASSERT(Path::mkdir("temp_subdir4")); // should work first time...
    CPPUNIT_ASSERT(Path::mkdir("temp_subdir4")); // ... and still return true the second time

    // creating a sub dir to an already existing dir should work though
    CPPUNIT_ASSERT(Path::mkdir("temp_subdir4/sub"));
    subdir_file = fopen("temp_subdir4/sub/subfile", "w");
    CPPUNIT_ASSERT(subdir_file);
    if(subdir_file) fclose(subdir_file);


    // cleanup
    CPPUNIT_ASSERT(Path::rmdir("temp_subdir4"));
}


void PathTestSuite::unicode()
{
    static const char unicodePath[] = u8"Äßéמש最終";
    // we try some funky utf8-stuff
    Path p(unicodePath);
    //OnScopeExit<std::function<void()> > deletePath([&]{Path::rmdir(p);});

    CPPUNIT_ASSERT(p.mkdir());
    CPPUNIT_ASSERT(p.isDir());

    bool changed = false;
    CPPUNIT_ASSERT(p.resolve(Path::RealPath, Path(), &changed));
    CPPUNIT_ASSERT(changed);

    Path parent = p + "/..";
    // check if the file exists
    bool exists = false;

    parent.visit([&](const Path &f_p) -> Path::VisitResult
                 {
                     if(f_p.name() == unicodePath)
                     {
                         exists = true;
                         return Path::Abort;
                     }
                     return Path::Continue;
                 });

    CPPUNIT_ASSERT(exists);

    CPPUNIT_ASSERT(Path::rmdir(p));

}

void PathTestSuite::renameFile()
{
    // delete rename target if it already exists:
    Path("testfile2").rm();

    FILE *testfile = fopen("testfile", "w");
    CPPUNIT_ASSERT(testfile != nullptr);
    fclose(testfile);

    Path pt = "testfile";
    CPPUNIT_ASSERT(pt.exists());
    CPPUNIT_ASSERT(pt.type() == Path::File);

    CPPUNIT_ASSERT(pt.rename("testfile2"));

    CPPUNIT_ASSERT(pt == "testfile2");
    CPPUNIT_ASSERT(pt.exists());
    CPPUNIT_ASSERT(pt.type() == Path::File);

    // old file should not exist anymore.
    testfile = fopen("testfile", "r");
    CPPUNIT_ASSERT(testfile == nullptr);

    testfile = fopen("testfile2", "r");
    CPPUNIT_ASSERT(testfile != nullptr);
    fclose(testfile);
}

void PathTestSuite::renameFileToExisting()
{
    FILE *testfile1 = fopen("testfile1", "w");
    FILE *testfile2 = fopen("testfile2", "w");

    CPPUNIT_ASSERT(testfile1 != nullptr);
    CPPUNIT_ASSERT(testfile2 != nullptr);

    fclose(testfile1);
    fclose(testfile2);

    Path p1("testfile1");
    CPPUNIT_ASSERT(p1.rename("testfile2"));
    CPPUNIT_ASSERT(p1.type() == Path::File);
    CPPUNIT_ASSERT(p1 == "testfile2");
}

void PathTestSuite::renameDir()
{
    // delete rename target if it already exists:
    Path::rmdir("testdir2");

    Path p("testdir");

    CPPUNIT_ASSERT(p.mkdir());
    CPPUNIT_ASSERT(p.exists());
    CPPUNIT_ASSERT(p.type() == Path::Directory);

    CPPUNIT_ASSERT(p.rename("testdir2"));

    CPPUNIT_ASSERT(p.exists());
    CPPUNIT_ASSERT(p.type() == Path::Directory);
    CPPUNIT_ASSERT(p == "testdir2");

    // old one should no longer exist.
    CPPUNIT_ASSERT(!Path("testdir").exists());
}

void PathTestSuite::renameDir_unicode()
{
    // delete rename target if it already exists:
    Path::rmdir(u8"Öüéמש最終");

    Path p(u8"Äßéמש最終");

    CPPUNIT_ASSERT(p.mkdir());
    CPPUNIT_ASSERT(p.exists());
    CPPUNIT_ASSERT(p.type() == Path::Directory);

    CPPUNIT_ASSERT(p.rename(u8"Öüéמש最終"));

    CPPUNIT_ASSERT(p.exists());
    CPPUNIT_ASSERT(p.type() == Path::Directory);
    CPPUNIT_ASSERT(p == u8"Öüéמש最終");

    // old one should no longer exist.
    CPPUNIT_ASSERT(!Path(u8"Äßéמש最終").exists());
}
