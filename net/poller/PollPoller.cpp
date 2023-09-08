#include "PollPoller.h"
#include <asm-generic/errno-base.h>
#include <cassert>
#include <cerrno>
#include <sys/poll.h>
#include "../utils/log.h"

PollPoller::PollPoller(EventLoop *loop)
    :Poller(loop)
{
    Logger::GetInstance()->debug("use PollPoller!");
}

PollPoller::~PollPoller() = default;


void PollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    //Todo:对std::vector<struct pollfd>取地址 只能得到std::vector<struct pollfd>*
    //struct pollfd * fds = &pollfds_;
    int numEvents = ::poll(&(*(pollfds_.begin())), pollfds_.size(), timeoutMs);
    int savedErrno = errno;
    if(numEvents>0)
    {
        Logger::GetInstance()->debug("PollPoller::poll {} events happened", numEvents);
        fillActiveChannels(numEvents, activeChannels);
    }
    else if(numEvents == 0)
    {
        Logger::GetInstance()->debug("PollPoller::poll nothing happen");
    }
    else
    {
        if(savedErrno != EINTR)
        {
            //errno = savedErrno;
            Logger::GetInstance()->error("PollPoller::poll error {} happen", std::strerror(savedErrno));
        }
    }
}

//把一个channel给加入poll监听
void PollPoller::updateChannel(Channel* channel)
{
    Poller::assertInLoopThread();
    int fd = channel->GetSocketFd();
    Logger::GetInstance()->debug("fd = {} events = {}", fd, channel->GetAllEvents());

    Channel::ChannelInPollerStatus inPollerStatus = channel->getInPollerStatus();
   
    if(fdInpollfdsIdx_.find(fd) == fdInpollfdsIdx_.end())
    {
        //new one, add it to pollfds_
        assert(channels_.find(fd) == channels_.end());
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = static_cast<short>(channel->GetAllEvents());//感兴趣的事件
        pfd.revents = 0;//接受到的事件

        //添加到pollfds_数组里
        pollfds_.push_back(pfd);
        // int idx = static_cast<int>(pollfds_.size())-1;
        // channel->set_index(idx);
        //纪录
        channels_[fd] = channel;
        channel->setInPollerStatus(Channel::ChannelInPollerStatus_KAdded);
        fdInpollfdsIdx_.insert({fd, pollfds_.size() - 1});
    }
    else
    {
        //update existed one
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);

        int idx = fdInpollfdsIdx_.at(fd);
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        
        struct pollfd& pfd = pollfds_[idx];
        assert(pfd.fd == fd ||
            pfd.fd == -(fd + 1)//负数表示在poller里面ignore处理
        );
        
        bool isIgoreInPoller = channel->IsNoneEvent();
        pfd.fd = !isIgoreInPoller ? fd : -(fd + 1);
        pfd.events = static_cast<short>(channel->GetAllEvents());
        pfd.revents = 0;
    }
    
}

//把一个channel给移除poll监听
void PollPoller::RemoveChannel(Channel* channel)
{
    Poller::assertInLoopThread();
    int fd = channel->GetSocketFd();
    Logger::GetInstance()->debug("RemoveChannel fd {} from epoller", fd);

    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->IsNoneEvent());

    assert(fdInpollfdsIdx_.find(fd) != fdInpollfdsIdx_.end());
    int idx = fdInpollfdsIdx_.at(fd);
    const struct pollfd& pfd = pollfds_[idx];
    assert(pfd.fd == -(fd + 1) && pfd.events == channel->GetAllEvents());

    //remove
    channels_.erase(fd);
    if (idx == pollfds_.size() - 1)
    {
        pollfds_.pop_back();
        
    }
    else
    {
        //删除中间的复杂度为O(n),可以转换思路，把要删除的元素和最后一个元素进行swap，然后把最后位置的元素给删除。
        int lastIdx = pollfds_.size()-1;
        int lastFd = pollfds_[lastIdx].fd;
        //swap the struct pollfd int pollfds_
        std::swap(pollfds_[lastIdx], pollfds_[idx]);
        //update the right pos in fdInpollfdsIdx_
        fdInpollfdsIdx_[lastFd] = idx;
        pollfds_.pop_back();
        
    }

    fdInpollfdsIdx_.erase(fd);
}


void PollPoller::fillActiveChannels(int numEvents, ChannelList* activeChanels) const
{
    //从头到尾的pollfds_的前numEvents个元素，为有事件的，后面的为无事件发生的。
    for(auto it = pollfds_.begin(); it != pollfds_.end() && numEvents > 0; ++it)
    {
        if(it->revents > 0)
        {
            --numEvents;
            assert(channels_.find(it->fd) != channels_.end());
            Channel* channel = channels_.at(it->fd);
            assert(channel);
            //设置发生的事件到channel对象上。
            channel->SetReceiveEvent(it->revents);
            //it->revents = 0;
            activeChanels->push_back(channel);
        }
    }
}


