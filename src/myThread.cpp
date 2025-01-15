#include "myThread.h"

MyThread::~MyThread() 
{
    stop();
}

int MyThread::start()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (NULL != _thread)
    {
        return -1;
    }

    mState = STATE_RUNNING;

    starting();

    _thread = new std::thread(entry, this);

    return 0;
}

int MyThread::stop()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (NULL != _thread)
    {
        stopping();
        _thread->join();
        delete _thread;
        _thread = NULL;
    }
    mState = STATE_UNSTART;

    return 0;
}

MyThread::THREAD_STATE_E MyThread::getState()
{
    return mState;
}

bool MyThread::isRunning()
{
    return (STATE_RUNNING == mState);
}

void MyThread::entry(void *opaque)
{
    if (opaque)
        ((MyThread *)opaque)->task();
}

void MyThread::task()
{
    run();
    mState = STATE_FINISHED;
}
