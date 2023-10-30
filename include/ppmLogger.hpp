#ifndef PPM_LOGGER_H
#define PPM_LOGGER_H

#include <string>
#include "spdlog/spdlog.h"

#include <iostream>

/**
 * @brief The PpmLogger class is intended to be an abstraction layer for the utilization
 * of the spdlog library and to make logging to a file and/or the console configurable.
 */
class PpmLogger {
    public:
        PpmLogger(std::string logname);

        void set_level(spdlog::level::level_enum level);
        void set_pattern(const std::string& pattern);
        
        void info(const std::string& message);
        void error(const std::string& message);
        void trace(const std::string& message);
        void critical(const std::string& message);
        void warn(const std::string& message);

        void flush();

    private:
        long LOG_SIZE = 1048576 * 5;                   ///> The size of a single log; these rotate.
        int LOG_NUM = 5;                               ///> The number of logs to rotate.
    
        spdlog::level::level_enum loglevel = spdlog::level::err;     ///> Log level for the log.
        
        std::shared_ptr<spdlog::logger> spdlogger;

        bool logToFileFlag;
        bool logToConsoleFlag;

        void setLogger(std::shared_ptr<spdlog::logger> spdlog_logger);
        void initializeFlagValuesFromEnvironment();
        const char* getEnvironmentVariable(std::string var);
        std::string toLowercase(std::string str);
        bool convertStringToBool(std::string str);
};

#endif