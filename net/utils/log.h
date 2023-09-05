#ifndef UTILS_LOG_H
#define UTILS_LOG_H

#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>


class Logger
{
public:
    static std::shared_ptr<spdlog::logger> GetInstance()
    {
        std::call_once(
            flag,
            [](){
                //logger_ = spdlog::basic_logger_mt("my_logger", "logs/my_log.txt", true);
                logger_ = spdlog::stdout_color_mt("console");
                logger_->set_level(spdlog::level::trace);
                logger_->set_pattern("[%H:%M:%S] [%^%l%$] %v");
            }
        );
        return logger_;
    }
    ~Logger()
    {
        //spdlog::shutdown();
    }
    static void log()
    {
               
    }

private:
    Logger() = default;
   
private:
    static std::once_flag flag;
    static std::shared_ptr<spdlog::logger> logger_;
};


#endif // !UTILS_LOG_H