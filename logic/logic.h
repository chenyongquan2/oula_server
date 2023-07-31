#ifndef LOGIC_LOGIC_H
#define LOGIC_LOGIC_H

#include <string>



class Logic
{
public:
    static bool Exec(const std::string& params);
};

//命令
class CmdBase
{
public:
    virtual ~CmdBase() {}//父类得虚析构函数。
    virtual bool Exec(const std::string& params) = 0;
};

class Cmd1: public CmdBase
{
public:
    virtual bool Exec(const std::string& params) override;
};

class Cmd2: public CmdBase
{
public:
    virtual bool Exec(const std::string& params) override;
};

class CmdDef: public CmdBase
{
public:
    virtual bool Exec(const std::string& params) override;
};


#endif // end LOGIC_LOGIC_H