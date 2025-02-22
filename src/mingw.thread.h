/*
 * mingw.thread.h
 *
 *  Created on: 05-Jul-2017
 *      Author: nisar
 */


#ifndef WIN32STDTHREAD_H
#define WIN32STDTHREAD_H

#include <windows.h>
#include <functional>
#include <memory>
#include <chrono>
#include <system_error>
#include <cerrno>
#include <process.h>

#ifdef _GLIBCXX_HAS_GTHREADS
#error This version of MinGW seems to include a win32 port of pthreads, and probably    \
    already has C++11 std threading classes implemented, based on pthreads.             \
    It is likely that you will get class redefinition errors below, and unfortunately   \
    this implementation can not be used standalone                                      \
    and independent of the system <mutex> header, since it relies on it for             \
    std::unique_lock and other utility classes. If you would still like to use this     \
    implementation (as it is more lightweight), you have to edit the                    \
    c++-config.h system header of your MinGW to not define _GLIBCXX_HAS_GTHREADS.       \
    This will prevent system headers from defining actual threading classes while still \
    defining the necessary utility classes.
#endif

//instead of INVALID_HANDLE_VALUE _beginthreadex returns 0
#define _STD_THREAD_INVALID_HANDLE 0
namespace std
{

class thread
{
public:
    class id
    {
        DWORD mId;
        void clear() {mId = 0;}
        friend class thread;
    public:
        explicit id(DWORD aId=0):mId(aId){}
        bool operator==(const id& other) const {return mId == other.mId;}
    };
protected:
    HANDLE mHandle;
    id mThreadId;
public:
    typedef HANDLE native_handle_type;
    id get_id() const noexcept {return mThreadId;}
    native_handle_type native_handle() const {return mHandle;}
    thread(): mHandle(_STD_THREAD_INVALID_HANDLE){}

    thread(thread&& other)
    :mHandle(other.mHandle), mThreadId(other.mThreadId)
    {
        other.mHandle = _STD_THREAD_INVALID_HANDLE;
        other.mThreadId.clear();
    }

    thread(const thread &other)=delete;

    template<class Function, class... Args>
    explicit thread(Function&& f, Args&&... args)
    {
        typedef decltype(std::bind(f, args...)) Call;
        Call* call = new Call(std::bind(f, args...));
        mHandle = (HANDLE)_beginthreadex(NULL, 0, threadfunc<Call>,
            (LPVOID)call, 0, (unsigned*)&(mThreadId.mId));
        if (mHandle == _STD_THREAD_INVALID_HANDLE)
        {
            int errnum = errno;
            delete call;
            throw std::system_error(errnum, std::generic_category());
        }
    }
    template <class Call>
    static unsigned int __stdcall threadfunc(void* arg)
    {
        std::unique_ptr<Call> upCall(static_cast<Call*>(arg));
        (*upCall)();
        return (unsigned long)0;
    }
    bool joinable() const {return mHandle != _STD_THREAD_INVALID_HANDLE;}
    void join()
    {
        if (get_id() == id(GetCurrentThreadId()))
            throw std::system_error(EDEADLK, std::generic_category());
        if (mHandle == _STD_THREAD_INVALID_HANDLE)
            throw std::system_error(ESRCH, std::generic_category());
        if (!joinable())
            throw std::system_error(EINVAL, std::generic_category());
        WaitForSingleObject(mHandle, INFINITE);
        CloseHandle(mHandle);
        mHandle = _STD_THREAD_INVALID_HANDLE;
        mThreadId.clear();
    }

    ~thread()
    {
        if (joinable())
            std::terminate();
    }
    thread& operator=(const thread&) = delete;
    thread& operator=(thread&& other) noexcept
    {
        if (joinable())
          std::terminate();
        swap(std::forward<thread>(other));
        return *this;
    }
    void swap(thread&& other) noexcept
    {
        std::swap(mHandle, other.mHandle);
        std::swap(mThreadId.mId, other.mThreadId.mId);
    }
    static unsigned int hardware_concurrency() noexcept
    {
        static int ncpus = -1;
        if (ncpus == -1)
        {
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            ncpus = sysinfo.dwNumberOfProcessors;
        }
        return ncpus;
    }
    void detach()
    {
        if (!joinable())
            throw std::system_error(EINVAL, std::generic_category());
        if (mHandle != _STD_THREAD_INVALID_HANDLE)
        {
            CloseHandle(mHandle);
            mHandle = _STD_THREAD_INVALID_HANDLE;
        }
        mThreadId.clear();
    }
};

namespace this_thread
{
    inline thread::id get_id() {return thread::id(GetCurrentThreadId());}
    inline void yield() {Sleep(0);}
    template< class Rep, class Period >
    void sleep_for( const std::chrono::duration<Rep,Period>& sleep_duration)
    {
        Sleep(std::chrono::duration_cast<std::chrono::milliseconds>(sleep_duration).count());
    }
    template <class Clock, class Duration>
    void sleep_until(const std::chrono::time_point<Clock,Duration>& sleep_time)
    {
        sleep_for(sleep_time-Clock::now());
    }
}

}
#endif // WIN32STDTHREAD_H
