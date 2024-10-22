#include "CurrentThread.h"

namespace WYXB
{
namespace CurrentThread
{

__thread int t_tid = 0;

void cacheTid()
{
    if(t_tid == 0)
    {
        t_tid = static_cast<int>(::syscall(SYS_gettid));
    }
}







}






}