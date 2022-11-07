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
        PpmLogger(std::string ilogname, std::string elogname);

        void set_info_level(spdlog::level::level_enum level);
        void set_error_level(spdlog::level::level_enum level);
        void set_info_pattern(const std::string& pattern);
        void set_error_pattern(const std::string& pattern);
        
        void info(const std::string& message);
        void error(const std::string& message);
        void trace(const std::string& message);
        void critical(const std::string& message);
        void warn(const std::string& message);

        void flush();

    private:
        static constexpr long ilogsize = 1048576 * 5;                   ///> The size of a single information log; these rotate.
        static constexpr long elogsize = 1048576 * 2;                   ///> The size of a single error log; these rotate.
        static constexpr int ilognum = 5;                               ///> The number of information logs to rotate.
        static constexpr int elognum = 2;                               ///> The number of error logs to rotate.
    
        spdlog::level::level_enum iloglevel = spdlog::level::trace;     ///> Log level for the information log.
        spdlog::level::level_enum eloglevel = spdlog::level::err;       ///> Log level for the error log.
        
        std::shared_ptr<spdlog::logger> ilogger;
        std::shared_ptr<spdlog::logger> elogger;

        bool logToFileFlag;
        bool logToConsoleFlag;

        void setInfoLogger(std::shared_ptr<spdlog::logger> spdlog_logger);
        void setErrorLogger(std::shared_ptr<spdlog::logger> spdlog_logger);
        void logToConsole(std::string message);
        void initializeFlagValuesFromEnvironment();
        const char* getEnvironmentVariable(std::string var);
        std::string toLowercase(std::string str);
        bool convertStringToBool(std::string str);
};

#endif