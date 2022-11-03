#include "../../include/ppmLogger.h"

#include "tool.hpp"

PpmLogger::PpmLogger() {
    // setup information logger.
    ilogger->set_pattern("[%C%m%d %H:%M:%S.%f] [%l] %v");
    ilogger->set_level( iloglevel );

    // setup error logger.
    elogger->set_level( iloglevel );
    elogger->set_pattern("[%C%m%d %H:%M:%S.%f] [%l] %v");
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

void PpmLogger::flush() {
    ilogger->flush();
    elogger->flush();
}

void PpmLogger::warn(const std::string& message) {
    elogger->warn(message);
}

void PpmLogger::set_info_level(spdlog::level::level_enum level) {
    ilogger->set_level( level );
}

void PpmLogger::set_error_level(spdlog::level::level_enum level) {
    elogger->set_level( level );
}