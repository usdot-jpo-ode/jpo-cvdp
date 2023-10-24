#include "ppmLogger.hpp"

PpmLogger::PpmLogger(std::string logname) {
    // pull in the file & console flags from the environment
    initializeFlagValuesFromEnvironment();
    
    // setup logger.
    std::vector<spdlog::sink_ptr> sinks;
    if (logToFileFlag) {
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logname, LOG_SIZE, LOG_NUM));
    }
    if (logToConsoleFlag) {
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
    }
    setLogger(std::make_shared<spdlog::logger>("log", begin(sinks), end(sinks)));
    set_level( loglevel );
    set_pattern("[%C%m%d %H:%M:%S.%f] [%l] %v");
}

void PpmLogger::set_level(spdlog::level::level_enum level) {
    spdlogger->set_level( level );
}

void PpmLogger::set_pattern(const std::string& pattern) {
    spdlogger->set_pattern( pattern );
}

void PpmLogger::info(const std::string& message) {
    spdlogger->info(message.c_str());
}

void PpmLogger::error(const std::string& message) {
    spdlogger->error(message.c_str());
}

void PpmLogger::trace(const std::string& message) {
    spdlogger->trace(message.c_str());
}

void PpmLogger::critical(const std::string& message) {
    spdlogger->critical(message.c_str());
}

void PpmLogger::warn(const std::string& message) {
    spdlogger->warn(message.c_str());
}

void PpmLogger::flush() {
    spdlogger->flush();
}

void PpmLogger::setLogger(std::shared_ptr<spdlog::logger> spdlog_logger) {
    spdlogger = spdlog_logger;
}

void PpmLogger::initializeFlagValuesFromEnvironment() {
    std::string logToFileFlagString = getEnvironmentVariable("PPM_LOG_TO_FILE");
    std::string logToConsoleFlagString = getEnvironmentVariable("PPM_LOG_TO_CONSOLE");
    logToFileFlag = convertStringToBool(logToFileFlagString);
    logToConsoleFlag = convertStringToBool(logToConsoleFlagString);

    if (!logToFileFlag && !logToConsoleFlag) {
        std::cout << "WARNING: PPM_LOG_TO_FILE and PPM_LOG_TO_CONSOLE are both set to false. No logging will occur." << std::endl;
    }
}

const char* PpmLogger::getEnvironmentVariable(std::string variableName) {
    char* variableValue = getenv(variableName.c_str());
    if (variableValue == NULL) {
        return "";
    }
    return variableValue;
}

std::string PpmLogger::toLowercase(std::string s) {
    int counter = 0;
    char c;
    while (s[counter]) {
        c = s[counter];
        s[counter] = tolower(c);
        counter++;
    }
    return s;
}

bool PpmLogger::convertStringToBool(std::string value) {
    std::string lowercaseValue = toLowercase(value);
    if (lowercaseValue == "true" || lowercaseValue == "1") {
        return true;
    }
    return false;
}