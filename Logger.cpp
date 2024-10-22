#include "Logger.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "Timestamp.h"
#include <map>
#include <fstream>



namespace WYXB
{
    
Logger& Logger::getInstance()
{
    static Logger logger;

    return logger;
}
    
void Logger::setLogLevel(LogLevel level)
{
    level_ = level;
}

void Logger::log(std::string msg)
{

    
    std::string level_str = "";
    switch (level_) {
            case DEBUG: 
                level_str =  "DEBUG";
                break;
            case INFO: 
                level_str =  "INFO";
                break;
            case ERROR: 
                level_str =  "ERROR";
                break;
            case FATAL: 
                level_str =  "FATAL";
                break;
            default: level_str =  "UNKNOWN";
        }
    std::string log_msg = "[" + Timestamp::now().toString() + "] [" + level_str + "] " + msg;
    
    // 根据日志级别选择正确的文件流进行输出
    std::ofstream& file_stream = getLogFileStream(level_);
    if (file_stream.is_open()) {
        file_stream << log_msg << std::endl;
    } else {
        file_stream.open(getLogFileName(level_));
        if (file_stream.is_open()) {
            file_stream << log_msg << std::endl;
        } else {
            std::cout << "Failed to open log file for level: " << level_str << std::endl;
        }
    }
    
}
    



std::string Logger::getLogFileName(LogLevel level) const 
{
    switch (level) {
        case DEBUG:
            return "debug.log";
        case INFO:
            return "info.log";
        case ERROR:
            return "error.log";
        case FATAL:
            return "fatal.log";
        default:
            return "unknown.log";
    }
}

std::ofstream& Logger::getLogFileStream(LogLevel level) {
    static std::map<LogLevel, std::ofstream> log_files;
    auto it = log_files.find(level);
    if (it == log_files.end()) {
        log_files[level].open(getLogFileName(level), std::ios::app);
        return log_files[level];
    }
    return it->second;
}


}

#ifdef LOG_TEST

int main()
{
    WYXB::Logger::getInstance().setLogLevel(WYXB::INFO);
    LOG_INFO("info log msg");
    LOG_INFO("info log msg");
    LOG_INFO("info log msg");
    LOG_DEBUG("debug log msg");
    LOG_ERROR("error log msg");
    LOG_FATAL("fatal log msg");

    return 0;
}



#endif