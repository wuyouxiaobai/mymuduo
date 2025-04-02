#include "Buffer.h"
#include <sys/uio.h>
#include <unistd.h>

namespace WYXB
{

// 从fd上读数据
ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    constexpr size_t extrabufSize = 65536;
    std::array<char, extrabufSize> extrabuf{}; // 64k
    struct iovec vec[2];

    const size_t writable = writableBytes();
    vec[0].iov_base = begin()  + writerIndex_;
    vec[0].iov_len  = writable;

    vec[1].iov_base = extrabuf.data();
    vec[1].iov_len = extrabuf.size();

    const int iovcnt = (writable < extrabufSize) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if(n < 0)
    {
        *saveErrno = errno;
    }
    else if(n <= writable)
    {
        writerIndex_ += n;
    }
    else
    {
        writerIndex_ = buffer_.size();
        append(extrabuf.data(), n - writable);
    }
    return n;
}

    // 通过fd发送数据
ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek().data(), readableBytes());
    if(n <= 0)
    {
        *saveErrno = errno;
    }
    return n;
}

}
