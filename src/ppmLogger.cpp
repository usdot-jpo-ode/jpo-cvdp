#include "ppmLogger.hpp"

PpmLogger::PpmLogger(std::string logname) {
    initializeFlagValuesFromEnvironment();
    
    std::vector<spdlog::sink_ptr> sinks;
    if (logToFileFlag) {
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logname, LOG_SIZE, LOG_NUM));
    }
    if (logToConsoleFlag) {
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
    }
    setLogger(std::make_shared<spdlog::logger>("log", begin(sinks), end(sinks)));

    setLogLevelFromEnvironment();
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

/**
 * @brief Sets the log level based on the PPM_LOG_LEVEL environment variable.
 * The log level is set to the default in the following cases:
 * - PPM_LOG_LEVEL is not set
 * - PPM_LOG_LEVEL is set to an unrecognized value (including empty string or number)
 */
void PpmLogger::setLogLevelFromEnvironment() {
    std::string logLevelString = getEnvironmentVariable("PPM_LOG_LEVEL");
    if (logLevelString == "") {
        std::cout << "WARNING: PPM_LOG_LEVEL is not set. Using default: " << getCurrentLogLevelAsString() << std::endl;
        return;
    }
    
    std::string lowercaseLogLevelString = toLowercase(logLevelString);
    if (lowercaseLogLevelString == "trace") {
        loglevel = spdlog::level::trace;
    }
    else if (lowercaseLogLevelString == "debug") {
        loglevel = spdlog::level::debug;
    }
    else if (lowercaseLogLevelString == "info") {
        loglevel = spdlog::level::info;
    }
    else if (lowercaseLogLevelString == "warn") {
        loglevel = spdlog::level::warn;
    }
    else if (lowercaseLogLevelString == "error") {
        loglevel = spdlog::level::err;
    }
    else if (lowercaseLogLevelString == "critical") {
        loglevel = spdlog::level::critical;
    }
    else {
        std::cout << "WARNING: PPM_LOG_LEVEL is set but is not recognized. Using default: " << getCurrentLogLevelAsString() << std::endl;
    }
    set_level( loglevel );
    info("Log level set to " + getCurrentLogLevelAsString());
}

/**
 * @brief Retrieves the value of an environment variable.
 * 
 * @param variableName The name of the environment variable to retrieve.
 * @return const char* The value of the environment variable or an empty string if the variable is not set.
 */
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

std::string PpmLogger::getCurrentLogLevelAsString() {
    std::string toReturn = "";
    if (loglevel == spdlog::level::trace) {
        toReturn = "TRACE";
    }
    else if (loglevel == spdlog::level::debug) {
        toReturn = "DEBUG";
    }
    else if (loglevel == spdlog::level::info) {
        toReturn = "INFO";
    }
    else if (loglevel == spdlog::level::warn) {
        toReturn = "WARN";
    }
    else if (loglevel == spdlog::level::err) {
        toReturn = "ERROR";
    }
    else if (loglevel == spdlog::level::critical) {
        toReturn = "CRITICAL";
    }
    else {
        toReturn = "UNKNOWN";
    }
    return toReturn;
}