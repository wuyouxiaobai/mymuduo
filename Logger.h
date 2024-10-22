#pragma once
#include <string>



// LOG_INFO("%s %s", "hello", "world") 
#define LOG_INFO(logmsgFmt, ...) \
    do{ \
        WYXB::Logger& logger = WYXB::Logger::getInstance(); \
        logger.setLogLevel(WYXB::INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFmt, ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0)

#define LOG_ERROR(logmsgFmt, ...) \
    do{ \
        WYXB::Logger& logger = WYXB::Logger::getInstance(); \
        logger.setLogLevel(WYXB::ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFmt, ##__VA_ARGS__); \
        logger.log(buf); \
   }while(0)


#define LOG_FATAL(logmsgFmt, ...) \
    do{ \
        WYXB::Logger& logger = WYXB::Logger::getInstance(); \
        logger.setLogLevel(WYXB::FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFmt, ##__VA_ARGS__); \
        logger.log(buf); \
        exit(1); \
    }while(0)

#define DEBUG_INFO
#ifdef DEBUG_INFO
#define LOG_DEBUG(logmsgFmt, ...) \
    do{ \
        WYXB::Logger& logger = WYXB::Logger::getInstance(); \
        logger.setLogLevel(WYXB::DEBUG); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFmt, ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0)
#endif

namespace WYXB
{

enum LogLevel
{
    INFO,
    ERROR,
    FATAL,
    DEBUG,
    
};


class Logger
{
private:
    LogLevel level_;
    
    std::string getLogFileName(LogLevel level) const;

    std::ofstream& getLogFileStream(LogLevel level);
    
    
    Logger(){}
public:
    static Logger& getInstance();
    
    void setLogLevel(LogLevel level);
    
    void log(std::string msg);

};

}