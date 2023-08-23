#include "Buffer.h"
#include <bits/types/struct_iovec.h>
#include <cstddef>
#include <sys/uio.h>

static constexpr size_t ExtraBufSize = 65535;

size_t Buffer::readFd(int fd)
{
    char extraBuf[ExtraBufSize];
    size_t writableSz = writableBytes();

    struct iovec vec[2];
    vec[0].iov_base = (void *)(begin()+writerIdx_);
    vec[0].iov_len = writableSz;
    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf);

    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    const int iocnt = (writableSz < sizeof(extraBuf)) ? 2 : 1;

    //readv()函数是用于从多个散布的内存块中读取数据的函数，节省进行系统调用的次数
    const size_t n = ::readv(fd, vec, iocnt);
    if(n<0)
    {

    }
    else if (n <= writableSz)
    {
        writerIdx_ += n;
    }
    else if(n > writableSz)
    {
        //writableSz is not enough

        //1 move the writerIdx_ to the last pos firstly
        writerIdx_ = buffer_.size();
        //2 append more (n - writableSz) bytes dynamic
        append(extraBuf, n-writableSz);
    }

    return n;
}