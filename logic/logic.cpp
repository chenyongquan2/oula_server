#include "logic.h"
#include <iostream>
#include <memory>
#include <thread>

using namespace std;

bool Logic::Exec(const string& params)
{
    if(params.empty())
    {
        std::cout << "[Logic::Exec] params.empty error" << std::endl;
        return false;
    }
    
    std::unique_ptr<CmdBase> spCmd;
    size_t sec = 0;
    //for(auto & cmd: params)
    char cmd = params[0];
    {
        switch (cmd) {
        case '1':
            spCmd = make_unique<Cmd1>();
            sec=1;
            break;
        case '2':
            spCmd = make_unique<Cmd2>();
            sec=2;
            break;
        default:
            spCmd = make_unique<CmdDef>();
            sec=3;
            break;
        }

        //模拟耗时的操作
        std::this_thread::sleep_for(std::chrono::seconds(sec));
        bool ret =  spCmd->Exec(params);
        if(!ret)
            return false;
    }
    
    return true;
}

bool Cmd1::Exec(const string& params)
{
    std::cout << "[Cmd1::Exec] Thread ID: " << std::this_thread::get_id() << ", params:" << params << std::endl;
    return true;
}

bool Cmd2::Exec(const string& params)
{
    std::cout << "[Cmd2::Exec] Thread ID: " << std::this_thread::get_id() << ", params:" << params << std::endl;
    return true; 
}

bool CmdDef::Exec(const string& params)
{
    std::cout << "[CmdDef::Exec] Thread ID: " << std::this_thread::get_id() << ", params:" << params << std::endl;
    return true;  
}
