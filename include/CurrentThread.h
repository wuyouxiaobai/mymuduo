#pragma once
#include <unistd.h>
#include <sys/syscall.h>


namespace WYXB
{
namespace CurrentThread
{

extern __thread int t_tid;

void cacheTid();

inline int tid()
{
    if(__builtin_expect(t_tid == 0, 0))
    {
        cacheTid();
    }
    return t_tid;
}




}



}