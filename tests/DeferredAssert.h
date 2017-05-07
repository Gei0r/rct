/*
 * rct's test suite uses cppunit with its CPPUNIT_ASSERT_* macros.
 * These macros have some problems:
 *
 * - A failing ASSERT terminates the program flow which might lead
 *   to a hanging test program (because another thread is waiting...).
 * - A failing ASSERT only prints that there was a failure, but not
 *   what the expected and the actual value IS, so that it's harder
 *   to debug a failing test.
 *
 * The class DeferredAsserter and the associated DEFERRED_ASSERT macro
 * solves these problems.
 */

#include <iostream>
#include <string>
#include <mutex>

#define DEFERRED_ASSERT(da, expr) (da).assert((expr), #expr, __LINE__);
#define DEFERRED_COMPARE(da, lhs, rhs) (da).compare(lhs, rhs, #lhs, #rhs, __LINE__)

class DeferredAsserter
{
public:
    DeferredAsserter() : m_testSuccessful(true) {}

    bool assert(bool value, const char* expr=nullptr, int line=0)
    {
        if(!value)
        {
            std::lock_guard<std::mutex> g(m_mutex);
            if(expr)
            {
                std::cerr << "TEST ERROR ";
                if(line) std::cerr << "(line " << line << ") ";
                std::cerr << expr << " is false";

                std::cerr << std::endl;
            }
            m_testSuccessful = false;
        }

        return value;
    }

    template<class L, class R>
    bool compare(const L &lhs, const R &rhs,
                 const char* expr_lhs=nullptr,
                 const char *expr_rhs=nullptr,
                 int line=0)
    {
        if(lhs != rhs)
        {
            std::lock_guard<std::mutex> g(m_mutex);
            if(expr_lhs && expr_rhs)
            {
                std::cerr << "TEST ERROR ";
                if(line) std::cerr << "(line " << line << ") ";
                std::cerr << expr_lhs << " (" << lhs << ") != "
                          << expr_rhs << " (" << rhs << ")";
                std::cerr << std::endl;
            }
            else
            {
                std::cerr << "TEST ERROR: "
                          << lhs << " != " << rhs << std::endl;
            }

            m_testSuccessful = false;
            return false;
        }

        return true;
    }

    void fail(const std::string &msg)
    {
        std::lock_guard<std::mutex> g(m_mutex);
        std::cerr << "TEST ERROR: " << msg << std::endl;
        m_testSuccessful = false;
    }

    bool result() {return m_testSuccessful;}

private:
    bool m_testSuccessful;
    std::mutex m_mutex;
};
