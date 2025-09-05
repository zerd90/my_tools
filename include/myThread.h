
#ifndef _MYTHREAD_H_
#define _MYTHREAD_H_

#include <functional>
#include <mutex>
#include <thread>

class MyThread
{
public:
    enum THREAD_STATE_E
    {
        STATE_UNSTART,
        STATE_RUNNING,
        STATE_FINISHED,
    };
    virtual ~MyThread();

    int  start();
    int  stop();
    void cancel();

    THREAD_STATE_E getState();
    bool           isRunning();

protected:
    virtual void   starting() {}
    virtual void   stopping() {}
    virtual void   run()  = 0;
    THREAD_STATE_E mState = STATE_UNSTART;

private:
    static void entry(void *opaque);

    void task();

    std::thread *_thread = nullptr;
    std::mutex   _mutex;
};

class ResourceGuard
{
public:
    ResourceGuard(const std::function<void()> &func) : _func(func) { _func(); }
    ~ResourceGuard()
    {
        if (!_dismiss)
            _func();
    }

    void dismiss() { _dismiss = true; }

private:
    bool                  _dismiss = false;
    std::function<void()> _func;
};

#endif