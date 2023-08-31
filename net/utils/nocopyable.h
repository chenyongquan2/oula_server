#ifndef NET_UTILS_NOCOPYABLE_H
#define NET_UTILS_NOCOPYABLE_H

class nocopyable
{
public:
    nocopyable(const nocopyable&) = delete;
    void operator=(const nocopyable&) = delete;

protected:
    nocopyable() = default;
    ~nocopyable()= default;
};

#endif //NET_UTILS_NOCOPYABLE_H