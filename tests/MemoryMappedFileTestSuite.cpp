#include "MemoryMappedFileTestSuite.h"

#include <rct/MemoryMappedFile.h>

#include <fstream>

void MemoryMappedFileTestSuite::mapSimpleFile()
{
    const std::string dataToWrite = "Hello world";
    {
        // create a file
        std::ofstream file("testfile.txt");
        file << dataToWrite;
    }

    MemoryMappedFile mmf("testfile.txt");

    CPPUNIT_ASSERT(mmf.isOpen());
    CPPUNIT_ASSERT(mmf.size() == dataToWrite.size());
    CPPUNIT_ASSERT(mmf.filename() == "testfile.txt");
    CPPUNIT_ASSERT(mmf.accessType() == MemoryMappedFile::READ_ONLY);

    std::string readData(static_cast<char*>(mmf.filePtr()), mmf.size());
    CPPUNIT_ASSERT(readData == dataToWrite);
}

void MemoryMappedFileTestSuite::nonExistingFile()
{
    MemoryMappedFile mmf1("fileDoesNotExist.txt");
    CPPUNIT_ASSERT(!mmf1.isOpen());
    CPPUNIT_ASSERT(mmf1.size() == 0);
    CPPUNIT_ASSERT(mmf1.filename().isEmpty());
    CPPUNIT_ASSERT(mmf1.accessType() == MemoryMappedFile::NO_ACCESS);
    mmf1.close();  // should be no-op

    MemoryMappedFile mmf2;  // standard ctor
    CPPUNIT_ASSERT(!mmf2.isOpen());
    CPPUNIT_ASSERT(mmf2.size() == 0);
    CPPUNIT_ASSERT(mmf2.accessType() == MemoryMappedFile::NO_ACCESS);
    CPPUNIT_ASSERT(!mmf2.open("fileDoesNotExist.txt"));
    CPPUNIT_ASSERT(!mmf2.isOpen());
    CPPUNIT_ASSERT(mmf2.size() == 0);
    CPPUNIT_ASSERT(mmf2.accessType() == MemoryMappedFile::NO_ACCESS);
}

void MemoryMappedFileTestSuite::closing()
{
    // Test whether MemoryMappedFile properly closes an opened file

    const std::string dataToWrite = "some data";
    {
        // create a file
        std::ofstream file("testfile.txt");
        file << dataToWrite << std::flush;
    }

    MemoryMappedFile mmf1("testfile.txt");
    CPPUNIT_ASSERT(mmf1.isOpen());

#ifdef _WIN32   // opening the same file multiple times is allowed on linux.
                // on windows, it's only allowed when it's explictly set up
                // like this in the CreateFile call.
                // Linux/Posix only has advisory, but no real mandatory locks.
    {
        // opening the file for writing should fail because the file is open
        std::ofstream fileWrite("testfile.txt");
        CPPUNIT_ASSERT(!fileWrite);
    }
#endif

    mmf1.close();

    CPPUNIT_ASSERT(!mmf1.isOpen());
    CPPUNIT_ASSERT(mmf1.size() == 0);
    CPPUNIT_ASSERT(mmf1.filename().isEmpty());
    CPPUNIT_ASSERT(mmf1.accessType() == MemoryMappedFile::NO_ACCESS);

#ifdef _WIN32
    {
        // opening the file for writing should work now, because the file
        // should have been closed.
        std::ofstream file("testfile.txt");
        CPPUNIT_ASSERT(file);
    }
#endif
}

void MemoryMappedFileTestSuite::moving()
{
    // try moving an empty file mapping
    MemoryMappedFile mmf1;
    CPPUNIT_ASSERT(!mmf1.isOpen());
    MemoryMappedFile mmf2(std::move(mmf1));
    CPPUNIT_ASSERT(!mmf1.isOpen());
    CPPUNIT_ASSERT(!mmf2.isOpen());



    // try moving an open file mapping
    const std::string dataToWrite = "some data";
    {
        // create a file
        std::ofstream file("testfile.txt");
        file << dataToWrite << std::flush;
    }

    MemoryMappedFile mmf3("testfile.txt");
    CPPUNIT_ASSERT(mmf3.isOpen());
    CPPUNIT_ASSERT(mmf3.size() == dataToWrite.size());
    CPPUNIT_ASSERT(mmf3.accessType() == MemoryMappedFile::READ_ONLY);
    MemoryMappedFile mmf4;
    CPPUNIT_ASSERT(!mmf4.isOpen());
    CPPUNIT_ASSERT(mmf4.accessType() == MemoryMappedFile::NO_ACCESS);

    mmf4 = std::move(mmf3);

    // mmf4 should now have the data of mmf3
    CPPUNIT_ASSERT(mmf4.isOpen());
    CPPUNIT_ASSERT(mmf4.size() == dataToWrite.size());
    CPPUNIT_ASSERT(mmf4.filename() == "testfile.txt");
    CPPUNIT_ASSERT(mmf4.accessType() == MemoryMappedFile::READ_ONLY);

    // mmf3 should now be empty.
    CPPUNIT_ASSERT(!mmf3.isOpen());
    CPPUNIT_ASSERT(mmf3.size() == 0);
    CPPUNIT_ASSERT(mmf3.filename().isEmpty());
    CPPUNIT_ASSERT(mmf3.accessType() == MemoryMappedFile::NO_ACCESS);
}

void MemoryMappedFileTestSuite::specialChars()
{
    MemoryMappedFile mmf(u8"testfile_Äßéמש最終.txt");

    CPPUNIT_ASSERT(mmf.isOpen());
    CPPUNIT_ASSERT(mmf.size() == 63);
    CPPUNIT_ASSERT(mmf.filename() == u8"testfile_Äßéמש最終.txt");

    const char *ptr = static_cast<const char*>(mmf.filePtr());

    std::string fileContent(ptr, mmf.size());

    CPPUNIT_ASSERT(fileContent ==
                   u8"This file has some utf-8 characters:\ntestfile_Äßéמש最終\n");
}