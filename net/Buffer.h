
#ifndef NET_BUFFER_H
#define NET_BUFFER_H

#include <cstddef>
#include <string>
#include <vector>
#include <assert.h>
#include <string.h>

/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buffer
{
public:
    //"cheap prepend" 可能表示在列表的开头或前面添加一个元素，
    //而这个操作的时间复杂度较低或占用的内存资源较少。
    //这种操作可以是通过修改指针或索引来实现，而不需要移动整个列表中的元素
    static constexpr size_t kCheapPrepend = 0;
    static constexpr size_t KInitSize=1024;
public:
    explicit Buffer(size_t initSize = KInitSize)
        :buffer_(initSize)
        ,readerIdx_(kCheapPrepend)
        ,writerIdx_(kCheapPrepend)
    {

    }
    //read data from socket, then append to the writerIdx_ pos in this buffer space.
    ssize_t readFd(int fd);
    
    size_t readableBytes() const 
    { return writerIdx_ - readerIdx_;}

    size_t writableBytes() const
    { return buffer_.size() - writerIdx_;}

    //prepend表示在前面添加[0,readerIdx]之间的区间
    size_t prependableBytes() const
    { return readerIdx_;}

    char* beginWrite() 
    { return begin() + writerIdx_;}

    void append(const char * data, size_t len)
    {
        //expand the space
        ensureWritableBytes(len);
        //copy data
        std::copy(data, data+len, beginWrite());//Todo stufy this.
        //move the wirterIdx pos
        hasWritten(len);
    }

    //move on the writerIdx_ pos.
    void hasWritten(size_t len)
    {
        assert(len <= writableBytes());
        writerIdx_+=len;
    }

    void ensureWritableBytes(size_t len)
    {
        if(writableBytes() < len)
        {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    void makeSpace(size_t len)
    {
        //判断可写的空间是否够
        size_t remainLen = writableBytes() + prependableBytes();
        if(remainLen < len + kCheapPrepend)
        {
            //the remain size it not enough, resize it
            // FIXME: move readable data
            buffer_.resize(writerIdx_ + len);//resize more len size data.
        }
        else
        {
            //move readable data to the front pos, make space inside buffer
            assert(kCheapPrepend < readerIdx_);
            size_t readable = readableBytes();

            std::copy(
                begin()+ readerIdx_,
                begin() + writerIdx_,
                begin() + kCheapPrepend //dest pos
            );
            readerIdx_= kCheapPrepend;
            writerIdx_=readerIdx_+readable;
            assert(readable==readableBytes());
        }
        
    }

    //"peek" 也常用于指查看数据结构中的元素或查看缓冲区中的内容，而不对其进行修改或删除。
    const char *peek() const
    {
        return begin() + readerIdx_;
    }

    //retrieve the date, move on the readerIdx_ pos
    void retrieve(size_t len)
    {
        size_t readableBytesNum = readableBytes();
        assert(len<=readableBytesNum);
        if(len<readableBytesNum)
        {
            readerIdx_+=len;
        }
        else if(len==readableBytesNum)
        {
            retrieveAll();
        }
    }

    //recover the readerIdx_ and writerIdx_ to the original value.
    void retrieveAll()
    {
        readerIdx_= kCheapPrepend;
        writerIdx_= kCheapPrepend;
    }

    //retrieve the data in buffer to string
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        assert(len<=readableBytes());
        std::string str(peek(),len);
        retrieve(len);
        return str;
    }



private:
    char *begin() 
        {return &(*(buffer_.begin()));}
    
    const char*begin() const 
        {return &(*(buffer_.begin()));}

private:
    size_t readerIdx_;//可读位置
    size_t writerIdx_;//可写位置
    std::vector<char> buffer_;
};

#endif //NET_BUFFER_H