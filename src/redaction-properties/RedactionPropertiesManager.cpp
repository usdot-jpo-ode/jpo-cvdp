#include "../../include/redaction-properties/RedactionPropertiesManager.hpp"

/**
 * @brief Construct a new Redaction Properties Manager object with a default path. Upon instantiation, fields are loaded from a file.
 * 
 */
RedactionPropertiesManager::RedactionPropertiesManager() {
    std::string rpmDebug = getEnvironmentVariable("RPM_DEBUG");
    if (rpmDebug == "true") {
        debug = true;
    }
    else {
        debug = false;
    }
    std::string path_to_fields_to_redact_file = getEnvironmentVariable("REDACTION_PROPERTIES_PATH");
    if (path_to_fields_to_redact_file == "") {
        log("REDACTION_PROPERTIES_PATH environment variable not set. Field redaction will not take place.");
        return;
    }
    loadFields(path_to_fields_to_redact_file); // load fields upon construction
}

/**
 * @brief Sets the debug flag for the class.
 * 
 * @param b boolean to set
 */
void RedactionPropertiesManager::setDebug(bool b) {
    debug = b;
}

/**
 * @brief Returns the file name that is used to persist the fields to redact.
 * 
 * @return string 
 */
std::string RedactionPropertiesManager::getFileName() {
    return fileName;
}

/**
 * @brief Sets the file name that is used to persist the fields to redact.
 * 
 * @param newFileName 
 */
void RedactionPropertiesManager::setFilename(std::string newFileName) {
    fileName = newFileName;
}

/**
 * @brief Returns a vector of the fields to redact.
 * 
 * @return vector<string> 
 */
std::vector<std::string> RedactionPropertiesManager::getFields() {
    return fieldsToRedact;
}

/**
 * @brief Returns the number of fields to redact.
 * 
 * @return int 
 */
int RedactionPropertiesManager::getNumFields() {
    return fieldsToRedact.size();
}

/**
 * @brief Checks if a field is in the list of fields to redact.
 * 
 * @param fieldToCheck 
 * @return boolean indicating whether or not the field is in the list
 */
bool RedactionPropertiesManager::isField(std::string fieldToCheck) {
    for (std::string field : fieldsToRedact) {
        if (field == fieldToCheck) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Adds a field to the list of fields to redact.
 * 
 * @param fieldToAdd 
 */
void RedactionPropertiesManager::addField(std::string fieldToAdd) {
    log("adding field " + fieldToAdd);
    fieldsToRedact.push_back(fieldToAdd);
}

/**
 * @brief Prints the fields. For debugging purposes.
 * 
 */
void RedactionPropertiesManager::printFields() {
    log("printing fields");
    std::cout << "=== Fields to Redact ===" << std::endl;
    for (std::string field: fieldsToRedact) {
        std::cout << field.c_str() << std::endl;
    }
}

/**
    * @brief Logs the message if the debug flag is set to true.
    * 
    * @param message 
    */
void RedactionPropertiesManager::log(std::string message) {
    if (debug) {
        std::cout << "[RPM] " << message.c_str() << std::endl;
    }
}

/**
    * @brief Loads the fields from a file.
    * 
    */
void RedactionPropertiesManager::loadFields(std::string fileName) {
    log("loading redaction fields");

    std::string line;            
    std::ifstream file(fileName);

    if (!file) {
        // file not found, no fields to redact
        log("The fieldsToRedact.txt file was not found. Field redaction will not take place.");
        return;
    }

    // for each line, read the line and insert it into fieldsToRedact
    while (getline(file,line)) {
        if (line.size() > 0) {
            fieldsToRedact.push_back(line);
        }
    }
    
    if (fieldsToRedact.size() > 0) {
        log("non-zero number of redaction fields loaded");
    }
    else {
        log("0 redaction fields loaded from file");
    }
}

const char* RedactionPropertiesManager::getEnvironmentVariable(const char* variableName) {
    const char* toReturn = getenv(variableName);
    if (!toReturn) {
        // fail silently
        toReturn = "";
    }
    return toReturn;
}