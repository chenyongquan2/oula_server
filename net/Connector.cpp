#include "Connector.h"
#include "Channel.h"
#include "Socket.h"
#include "SocketHelper.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cerrno>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include "utils/log.h"
#include <cassert>
#include "Eventloop.h"

static const int kMaxRetryDelayMs = 30*1000;
static const int kInitRetryDelayMs = 500;

Connector::Connector(EventLoop* eventloop, const InetAddress& serverAddr)
    :eventloop_(eventloop)
    ,serverAddr_(serverAddr)
    ,enableConnect_(false)
    ,state_(kDisconnected)
    ,retryDelayMs_(kInitRetryDelayMs)
{
    Logger::GetInstance()->debug("Connector constructor");
}
Connector::~Connector()
{
    Logger::GetInstance()->debug("Connector destructor");
    assert(!channel_);
}

// can be called in any thread
void Connector::start()
{
    enableConnect_= true;
    eventloop_->runInLoop(
        std::bind(&Connector::startInLoop,this)
    );
}

void Connector::startInLoop()
{
    eventloop_->assertInLoopThread();
    assert(state_== kDisconnected);
    if(enableConnect_)
    {
        connect();
    }
    else
    {
        Logger::GetInstance()->debug("has connected before");
    }
}

void Connector::connect()
{
    //创建非阻塞的socket(每次都得创建新的socket!!!)
    int sockfd = SocketHelper::CreateNonblockingOrDie(serverAddr_.family());
    const struct sockaddr* addr = serverAddr_.getSockAddr();

    // 在默认情况下，connect函数是阻塞的。当调用connect函数时，它将会阻塞当前线程，直到连接成功建立或发生错误。
    // 在阻塞模式下，connect函数会一直等待，直到连接成功或发生错误。如果连接成功建立，connect函数将返回0。如果发生错误，connect函数将返回-1，并设置相应的错误码（可以通过errno全局变量获取）来指示错误的原因。
    // 阻塞模式的connect函数在连接建立期间会一直等待，直到连接成功或发生错误。这意味着，如果网络延迟较高或目标主机无响应，connect函数的执行可能会有一定的延迟。
    // 如果你希望将connect函数设置为非阻塞模式，可以在调用connect函数之前将套接字设置为非阻塞模式。这可以通过调用fcntl函数或使用O_NONBLOCK标志来实现。在非阻塞模式下，connect函数将立即返回，并且连接的建立将在后台进行。
    // 你可以使用select、poll或epoll等多路复用函数来检查连接的状态。

    int ret = ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    
    //connect返回后，不一定一定成功，得先判断一下errno
    int saveError = (ret == 0)? 0: errno;
    //judge the errno
    switch (saveError) {
    // 这些枚举值是在调用::connect函数时可能返回的错误代码。它们的含义如下：
    // EINPROGRESS：表示连接正在进行中，但尚未完成。这通常发生在非阻塞套接字上，当调用::connect时，连接不能立即建立时，就会返回这个错误代码。
    // EINTR：表示系统调用被中断。这通常发生在信号处理程序中，当一个信号被捕获并且中断了系统调用时，就会返回这个错误代码。
    // EISCONN：表示套接字已经连接。这通常发生在多次调用::connect函数时，当套接字已经连接时，就会返回这个错误代码。
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        //进入下一个阶段,检查是否channel可写(因为accept是非阻塞的)
        connecting(sockfd);
        break;
    
    // EAGAIN：表示资源暂时不可用。这通常发生在非阻塞套接字上，当调用::connect时，连接不能立即建立时，就会返回这个错误代码。
    // EADDRINUSE：表示地址已经被使用。这通常发生在尝试绑定一个已经被其他进程使用的地址时，就会返回这个错误代码。
    // EADDRNOTAVAIL：表示地址不可用。这通常发生在尝试绑定一个不存在的地址时，就会返回这个错误代码。
    // ECONNREFUSED：表示连接被拒绝。这通常发生在远程主机拒绝连接请求时，就会返回这个错误代码。
    // ENETUNREACH：表示网络不可达。这通常发生在无法到达目标网络时，就会返回这个错误代码。
    case EAGAIN:
    //与accept不同的是,EAGAIN是真的错误，表示ephemeal port暂时用完了,要关闭socker再延期重试
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;
    
    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        Logger::GetInstance()->debug("connect error in Connector::startInLoop{} ", saveError);
        ::close(sockfd);
        break;
    default:
        Logger::GetInstance()->debug("Unexpected error in Connector::startInLoop{} ", saveError);
        ::close(sockfd);
        // connectErrorCallback_();
        break;
    }
    
    Logger::GetInstance()->debug("finish connecting!");
}

void Connector::connecting(int sockfd)
{
    //要确保连接已经完全建立成功，可以使用select或poll等多路复用函数来监听套接字的可写事件（POLLOUT或WRITE事件）。
    //当套接字变为可写时，才表示连接已经建立成功.

    setState(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(eventloop_, sockfd));
    channel_->SetWirteCallback(
        std::bind(&Connector::handleWrite,this)
    );
    // channel_->setErrorCallback(
    //   std::bind(&Connector::handleError, this));
    channel_->EnableWriteEvent();
    //等待channel上的socket可写，才标记着非阻塞的connect完全成功!
}

void Connector::handleWrite()
{
    Logger::GetInstance()->debug("[Connector::handleWrite]");
    if(state_ == kConnecting)
    {
        //已经连接上了，不再需要channel来handle判断可写事件啦!避免一直触发。
        int sockfd = removeAndResetChannel();
        int err = SocketHelper::getSocketError(sockfd);
        if(err)
        {
            //errno is not thread safe?
            Logger::GetInstance()->warn("[Connector::handleWrite] SO_ERROR={}",std::strerror(err));
            //connect not yet，retry it
            retry(sockfd);
        }

        // 要处理自连接(self-connection)。出现这种状况的原因如下。在发起连接的时
        // 候，TCP/IP协议栈会先选择source IP和source port,.在没有显式调用bind(2)
        // 的情况下，source IP由路由表确定，source port由TCP/IP协议栈从local port
        // range8中选取尚未使用的port(即ephemeral port)。如果destination IP正好
        // 是本机，而destination port位于local port range,且没有服务程序监听的话，
        // ephemeral port可能正好选中了destination port,这就出现（source IP,source
        // port)=(destination IP,destination port)的情况，即发生了自连接。处理办法是
        // 断开连接再重试，否则原本侦听destination port的服务进程也无法启动了。
        else if(SocketHelper::isSelfConnect(sockfd))
        {
            //不应该出现自连接
            Logger::GetInstance()->warn("[Connector::handleWrite] isSelfConnect!!!");
            retry(sockfd);
        }
        else
        {
            //一切都ok，正式完成连接！
            setState(kConnected);
            if(enableConnect_)
            {
                if(newConnectionCallback_)
                {
                    //执行TcpClient的回调 即 TcpClient::newConnection
                    newConnectionCallback_(sockfd);
                }
            }
            else
            {
                //说明已经stop start了
                ::close(sockfd);
            }
        }
    }
    else
    {
        Logger::GetInstance()->error("[Connector::handleWrite] state_ != kConnecting!!!");
        assert(state_==kDisconnected);
    }
}

void Connector::handleError()
{
    
}

// must be called in loop thread
void Connector::restart()
{
    eventloop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    enableConnect_=true;
    startInLoop();
}

// can be called in any thread
void Connector::stop()
{
    enableConnect_ = false;
    eventloop_->queueInLoop(
        std::bind(&Connector::stopInLoop, this)
    );
}

void Connector::stopInLoop()
{
    eventloop_->assertInLoopThread();
    if(state_ == kConnecting)
    {
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    //socket是一次性的，一旦出错(比如对方拒绝连接)，就无法恢复，只能关闭重来,
    //但是Connector是可以反复使用的。
    //先关闭旧的连接符，然后去startInLoop生成新的sockfd
    int ret = ::close(sockfd);
    if(ret < 0)
    {
        Logger::GetInstance()->error("[Connector::retry] close {} failed", sockfd);
    }
    setState(kDisconnected);
    if(enableConnect_)
    {
        Logger::GetInstance()->debug("Connector::retry - Retry connecting to {} in {} ms"
            ,serverAddr_.port(), retryDelayMs_
        );
        eventloop_->runAfter(retryDelayMs_ / 1000.0,
            std::bind(&Connector::startInLoop, shared_from_this()));
        
        //重试的时间间隔已经逐渐延长，即back-off
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else
    {
        Logger::GetInstance()->debug("do not connect");
    }

}

int Connector::removeAndResetChannel()
{
    channel_->DisableAllEvent();
    channel_->remove();
    int sockfd = channel_->GetSocketFd();
    // Can't reset channel_ here, because we are inside Channel::handleEvent
    eventloop_->queueInLoop(std::bind(
        &Connector::resetChannel,this
    ));
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();
}