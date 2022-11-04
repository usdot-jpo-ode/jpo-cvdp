#include "../../include/ppmLogger.hpp"

PpmLogger::PpmLogger(std::string ilogname, std::string elogname) {
    // setup information logger.
    setInfoLogger(spdlog::rotating_logger_mt("ilog", ilogname, ilogsize, ilognum));
    set_info_level( iloglevel );
    set_info_pattern("[%C%m%d %H:%M:%S.%f] [%l] %v");

    // setup error logger.
    setErrorLogger(spdlog::rotating_logger_mt("elog", elogname, elogsize, elognum));
    set_error_level( eloglevel );
    set_error_pattern("[%C%m%d %H:%M:%S.%f] [%l] %v");
}

void PpmLogger::set_info_level(spdlog::level::level_enum level) {
    ilogger->set_level( level );
}

void PpmLogger::set_error_level(spdlog::level::level_enum level) {
    elogger->set_level( level );
}

void PpmLogger::set_info_pattern(const std::string& pattern) {
    ilogger->set_pattern( pattern );
}

void PpmLogger::set_error_pattern(const std::string& pattern) {
    elogger->set_pattern( pattern );
}

void PpmLogger::info(const std::string& message) {
    if (logToFileFlag) {
        ilogger->info(message.c_str());
    }
    if (logToConsoleFlag) {
        logToConsole("[INFO] " + message);
    }
}

void PpmLogger::error(const std::string& message) {
    if (logToFileFlag) {
        elogger->error(message.c_str());
    }
    if (logToConsoleFlag) {
        logToConsole("[ERROR] " + message);
    }
}

void PpmLogger::trace(const std::string& message) {
    if (logToFileFlag) {
        ilogger->trace(message.c_str());
    }
    if (logToConsoleFlag) {
        logToConsole("[TRACE] " + message);
    }
}

void PpmLogger::critical(const std::string& message) {
    if (logToFileFlag) {
        elogger->critical(message.c_str());
    }
    if (logToConsoleFlag) {
        logToConsole("[CRITICAL] " + message);
    }
}

void PpmLogger::warn(const std::string& message) {
    if (logToFileFlag) {
        elogger->warn(message.c_str());
    }
    if (logToConsoleFlag) {
        logToConsole("[WARN] " + message);
    }
}

void PpmLogger::flush() {
    ilogger->flush();
    elogger->flush();
}

void PpmLogger::setInfoLogger(std::shared_ptr<spdlog::logger> spdlog_logger) {
    ilogger = spdlog_logger;
}

void PpmLogger::setErrorLogger(std::shared_ptr<spdlog::logger> spdlog_logger) {
    elogger = spdlog_logger;
}

void PpmLogger::logToConsole(std::string message) {
    // prepare datetime string
    time_t dateTime = time(0);
    char* dateTimeString = ctime(&dateTime);
    dateTimeString[strlen(dateTimeString) - 1] = '\0';

    // print message to standard output
    std::cout << "[" << dateTimeString << "] " << message << std::endl;
}