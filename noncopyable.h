#pragma once

namespace WYXB
{

/**
 * noncopyable被私有继承后，派生对象能正常构造和析构但是不能拷贝构造和赋值
 */

class noncopyable
{
private:

public:
    noncopyable(const noncopyable&)=delete;
    noncopyable& operator=(const noncopyable&)=delete;
protected:
    noncopyable()=default;
    ~noncopyable()=default;
};























    
}