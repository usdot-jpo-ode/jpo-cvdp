#include "../../include/ppmLogger.hpp"

void PpmLogger::setInfoLogger(std::shared_ptr<spdlog::logger> spdlog_logger) {
    ilogger = spdlog_logger;
}

void PpmLogger::setErrorLogger(std::shared_ptr<spdlog::logger> spdlog_logger) {
    elogger = spdlog_logger;
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
    ilogger->info(message);
}

void PpmLogger::error(const std::string& message) {
    elogger->error(message);
}

void PpmLogger::trace(const std::string& message) {
    ilogger->trace(message);
}

void PpmLogger::critical(const std::string& message) {
    elogger->critical(message);
}

void PpmLogger::warn(const std::string& message) {
    elogger->warn(message);
}

void PpmLogger::flush() {
    ilogger->flush();
    elogger->flush();
}