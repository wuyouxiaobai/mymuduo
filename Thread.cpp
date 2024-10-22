#include "Thread.h"
#include "CurrentThread.h"
#include <semaphore.h>


namespace WYXB
{

std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string& name)
    : started_(false),
      joined_(false),
      func_(func),
      name_(name),
      tid_(0)
{
    
}
Thread::~Thread()
{
    if(started_ && !joined_)
    {

        thread_->detach();
    }
}



void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    // 创建线程
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        // 获得线程tid值
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        // 执行函数
        func_();
    }));

    // 等待线程启动后获得上面创建的线程tid值
    sem_wait(&sem);
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if(name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}


}

