#pragma once
#include <string_view>
#include <vector>
#include <cstddef>
#include <string>
#include <algorithm>
#include <cstring>

namespace WYXB
{

// 网络库底层缓冲区类
class Buffer
{
public:
    static constexpr size_t kCheapPrepend = 8;  // 预留空间大小
    static constexpr size_t kInitialSize = 1024;  // 初始缓冲区大小

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(initialSize + kCheapPrepend),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend)
    {}
    
    [[nodiscard]] size_t readableBytes() const
    {
        return writerIndex_ - readerIndex_;
    }
    
    [[nodiscard]] size_t writableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }
    
    [[nodiscard]] size_t prependableBytes() const
    {
        return readerIndex_;
    }

    [[nodiscard]] std::string_view peek() const
    {
        return {buffer_.data() + readerIndex_, readableBytes()};
    }

    // 复位缓冲区
    void retrieve(size_t len)
    {
        readerIndex_ = std::clamp(readerIndex_ + len, readerIndex_, writerIndex_);
        if (readerIndex_ == writerIndex_) 
        {
            retrieveAll();
        }
    }
    
    void retrieveAll()
    {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    // 将onMessage（）函数上报的Buffer数据，转成string数据返回
    [[nodiscard]] std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    [[nodiscard]] std::string retrieveAsString(size_t len)
    {
        std::string result(peek().substr(0, len));
        retrieve(len);
        return result;
    }

    void ensureWritableBytes(size_t len)
    {
        if(writableBytes() < len)
        {
            makeSpace(len);
        }
    }

    // 把[data, data+len]的内容追加到缓冲区
    void append(const char* data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        writerIndex_ += len;
    }

    [[nodiscard]] char* beginWrite()
    {
        return begin() + writerIndex_;
    } 

    // 从fd上读数据
    [[nodiscard]] ssize_t readFd(int fd, int* saveErrno);
    // 通过fd发送数据
    [[nodiscard]] ssize_t writeFd(int fd, int* saveErrno);

private:

    [[nodiscard]] char* begin()
    {
        return buffer_.data();
    }

    [[nodiscard]] const char* begin() const
    {
        return buffer_.data();
    }

    void makeSpace(size_t len)
    {
        if(writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::memmove(buffer_.data() + kCheapPrepend, buffer_.data() + readerIndex_, readable);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = kCheapPrepend + readable;
        }
    }

    std::vector<char> buffer_;  // 缓冲区数据
    size_t readerIndex_;         // 读指针位置
    size_t writerIndex_;         // 写指针位置




};


}